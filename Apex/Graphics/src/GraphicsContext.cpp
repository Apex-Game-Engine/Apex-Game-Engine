#include "Graphics/GraphicsContext.h"

#include <xxhash.h>

#include "Graphics/Vulkan/VulkanContext.h"
#include "Memory/AxPool.h"
#include "Memory/MemoryManager.h"

namespace apex::gfx {

	Unknown::~Unknown() = default;

	static Context* CreateContext(Api api)
	{
		Context* instance = nullptr;
		switch (api)
		{
		case Api::None:
			// instance = new NullContext;
			break;
		case Api::Vulkan:
		{
			instance = apex_new VulkanContext;
			break;
		}
		case Api::D3D12:
			// instance = new D3D12Context;
			break;
		case Api::WebGpu:
			// instance = new WebGpuContext;
			break;
		}
		axCriticalAssertFmt(instance, "Failed to create a graphics context! Check if the requested API is available on the devices.");
		return instance;
	}

	namespace detail {
		static Context* g_context = nullptr;
		enum StageDestination { eStageDstBuffer, eStageDstImage };
		struct StagingDesc
		{
			StageDestination dstType;
			BufferRef stagingBuffer;
			union
			{
				Buffer* dstBuffer;
				Image* dstImage;
			};
		};
		static AxArray<StagingDesc> g_stagingBatch;
	}

	u64 VertexFormat::CalcHash() const
	{
		return XXH3_64bits(attributes, sizeof(attributes));
	}

	bool VertexFormat::Validate() const
	{
		u32 streamMax = 0, locationMax = 0;
		bool uniqueStreams[VERT_ATTR_STREAM_MAX] = {};
		bool uniqueLocations[VERT_ATTR_LOCATION_MAX] = {};
		for (auto attr : attributes)
		{
			streamMax = max(streamMax, attr.stream);
			locationMax = max(locationMax, attr.location);
			uniqueStreams[attr.stream] = true;
			uniqueLocations[attr.location] = true;
		}
		for (u32 i = 0; i < streamMax + 1; i++)
		{
			if (!uniqueStreams[i])
				return false;
		}
		for (u32 i = 0; i < locationMax + 1; i++)
		{
			if (!uniqueLocations[i])
				return false;
		}
		return true;
	}

	u32 VertexFormat::CalcVertexStride() const
	{
		u32 stride = 0;
		for (auto attr : attributes)
		{
			stride += Factory::GetFormatSize(static_cast<Format>(attr.format));
		}
		return stride;
	}

	u32 VertexFormat::CalcStreamCount() const
	{
		u32 streamMax = 0;
		for (auto attr : attributes)
		{
			streamMax = max(streamMax, attr.stream);
		}
		return streamMax + 1;
	}

	void VertexFormat::RetrieveStreams(AxSpan<VertexAttributeStream> streams) const
	{
		axAssert(streams.data() && streams.size() >= CalcStreamCount());
		for (auto& attr : attributes)
		// for (u32 i = 0; i < std::size(attributes); i++)
		{
			// auto& attr = attributes[i];
			streams[attr.stream].stride += Factory::GetFormatSize(static_cast<Format>(attr.format));
		}
		u32 offset = 0;
		for (auto& stream : streams)
		{
			stream.offset = offset;
			offset += stream.stride;
		}
	}

	bool VertexFormat::RetrieveStreamAtIndex(u32 stream_index, VertexAttributeStream& out_stream) const
	{
		bool found = false;
		out_stream.offset = Constants::u32_MAX;
		out_stream.stride = 0;
		for (auto attr : attributes)
		{
			if (attr.stream == stream_index)
			{
				found = true;
				out_stream.stride += Factory::GetFormatSize(static_cast<Format>(attr.format));
			}
			else if (attr.stream < stream_index)
			{
				out_stream.offset += Factory::GetFormatSize(static_cast<Format>(attr.format));
			}
		}
		return found;
	}

	u32 VertexFormat::CalcAttributeCount() const
	{
		u32 count = 0;
		for (const auto attr : attributes)
		{
			if (attr.IsValid())
			{
				count++;
			}
		}
		return count;
	}

#define DEFINE_VERTEX_ATTRIBUTE_STRING(ATTR) #ATTR

	constexpr const char* VertexAttribute_strings[VertexAttribute::eMAXCOUNT] = {
		FOREACH_VERTEX_FORMAT_ATTRIBUTES(DEFINE_VERTEX_ATTRIBUTE_STRING, COMMA)
	};

	constexpr const char* Format_strings[(size_t)Format::COUNT] = {
		"UNDEFINED",
		"R8_UNORM", "R8_SNORM", "R8_UINT", "R8_SINT", "R8_SRGB",
		"R8G8_UNORM", "R8G8_SNORM", "R8G8_UINT", "R8G8_SINT", "R8G8_SRGB",
		"R8G8B8_UNORM", "R8G8B8_SNORM", "R8G8B8_UINT", "R8G8B8_SINT", "R8G8B8_SRGB",
		"B8G8R8_UNORM", "B8G8R8_SNORM", "B8G8R8_UINT", "B8G8R8_SINT", "B8G8R8_SRGB",
		"R8G8B8A8_UNORM", "R8G8B8A8_SNORM", "R8G8B8A8_UINT", "R8G8B8A8_SINT", "R8G8B8A8_SRGB",
		"B8G8R8A8_UNORM", "B8G8R8A8_SNORM", "B8G8R8A8_UINT", "B8G8R8A8_SINT", "B8G8R8A8_SRGB",
		"R16_UNORM", "R16_SNORM", "R16_UINT", "R16_SINT", "R16_SFLOAT",
		"R16G16_UNORM", "R16G16_SNORM", "R16G16_UINT", "R16G16_SINT", "R16G16_SFLOAT",
		"R16G16B16_UNORM", "R16G16B16_SNORM", "R16G16B16_UINT", "R16G16B16_SINT", "R16G16B16_SFLOAT",
		"R16G16B16A16_UNORM", "R16G16B16A16_SNORM", "R16G16B16A16_UINT", "R16G16B16A16_SINT", "R16G16B16A16_SFLOAT",
		"R32_UINT", "R32_SINT", "R32_SFLOAT", "R32G32_UINT", "R32G32_SINT", "R32G32_SFLOAT",
		"R32G32B32_UINT", "R32G32B32_SINT", "R32G32B32_SFLOAT",
		"R32G32B32A32_UINT", "R32G32B32A32_SINT", "R32G32B32A32_SFLOAT",
		"B10G11R11_UFLOAT",
		"D16_UNORM", "D32_SFLOAT", "S8_UINT", "D24_UNORM_S8_UINT",
	};

	const char* VertexFormat::ToString(char* const buf, u32 bufsz)
	{
		auto result = fmt::format_to_n(buf, bufsz, "");
		for (u32 i = 0; i < std::size(attributes); ++i)
		{
			VertexAttribute& attr = attributes[i];
			if (attr.IsValid())
			{
				result = fmt::format_to_n(result.out, bufsz ,
					"attr: <{0} {1} {2} {3} {4}>\n",
					VertexAttribute_strings[i],
					(u32)attr.location,
					(u32)attr.stream,
					(u32)attr.offset,
					Format_strings[(u32)attr.format]);
				bufsz -= result.size - 1;
			}
		}
		return buf;
	}

	void Factory::InitContext(Api api, plat::PlatformWindow& window)
	{
		detail::g_context = CreateContext(api);
		detail::g_context->Init(window);
	}

	void Factory::ShutdownContext()
	{
		detail::g_stagingBatch.reset();
		detail::g_context->Shutdown();
		delete detail::g_context;
		detail::g_context = nullptr;
	}

	Context& Factory::GetContext()
	{
		return *detail::g_context;
	}

	GraphicsPipeline* Factory::CreateGraphicsPipeline(const char* name, const GraphicsPipelineCreateDesc& desc)
	{
		return GetContext().GetDevice()->CreateGraphicsPipeline(name, desc);
	}

	ComputePipeline* Factory::CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc)
	{
		return GetContext().GetDevice()->CreateComputePipeline(name, desc);
	}

	Buffer* Factory::CreateBuffer(const char* name, BufferCreateDesc const& desc)
	{
		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}

	Buffer* Factory::CreateVertexBuffer(const char* name, size_t size, const void* initial_data)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Vertex | BufferUsageFlagBits::TransferDst,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.preferredFlags = MemoryPropertyFlagBits::None,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false,
			.alignment = 0,
			.pInitialData = initial_data
		};

		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}

	Buffer* Factory::CreateIndexBuffer(const char* name, size_t size, const void* initial_data)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Index | BufferUsageFlagBits::TransferDst,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.preferredFlags = MemoryPropertyFlagBits::None,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false,
			.alignment = 0,
			.pInitialData = initial_data
		};

		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}

	Buffer* Factory::CreateStagingBuffer(const char* name, size_t size)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::TransferSrc,
			.requiredFlags = MemoryPropertyFlagBits::HostCoherent,
			.preferredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessSequential,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}
	
	Buffer* Factory::CreateUniformBuffer(const char* name, size_t size)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Uniform,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal |
							 MemoryPropertyFlagBits::HostVisible |
							 MemoryPropertyFlagBits::HostCoherent,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessSequential,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}

	Buffer* Factory::CreateStorageBuffer(const char* name, size_t size)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Storage,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal |
							 MemoryPropertyFlagBits::HostVisible |
							 MemoryPropertyFlagBits::HostCoherent,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessSequential,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		return GetContext().GetDevice()->CreateBuffer(name, desc);
	}

	Image* Factory::CreateImage(const char* name, ImageCreateDesc const& desc)
	{
		return GetContext().GetDevice()->CreateImage(name, desc);
	}

	Image* Factory::CreateDepthImage(const char* name, Dim2D dimensions, Format format)
	{
		const ImageCreateDesc desc {
			.imageType = ImageType::Image2D,
			.format = format,
			.usageFlags = ImageUsageFlagBits::DepthStencilAttachment,
			.dimensions = { dimensions.width, dimensions.height },
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false
		};

		return GetContext().GetDevice()->CreateImage(name, desc);
	}

	Image* Factory::CreateTexture2D(const char* name, Dim2D dimensions, Format format)
	{
		const ImageCreateDesc desc {
			.imageType = ImageType::Image2D,
			.format = format,
			.usageFlags = ImageUsageFlagBits::Sampled | ImageUsageFlagBits::TransferDst,
			.dimensions = { dimensions.width, dimensions.height },
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false,
		};

		return GetContext().GetDevice()->CreateImage(name, desc);
	}

	ImageView* Factory::CreateImageView(const char* name, ImageViewCreateDesc const& desc)
	{
		return GetContext().GetDevice()->CreateImageView(name, desc);
	}

	void Factory::CopyBufferToBuffer(Buffer* dst_buffer, Buffer* src_buffer)
	{

	}

	void Factory::CopyBufferToImage(Image* dst_image, Buffer* src_buffer)
	{
	}

	void Factory::EnqueueBufferUpload(Buffer* dst_buffer, const ByteArrayRef& data)
	{
		Buffer* stagingBuffer = CreateStagingBuffer("buffer-staging-buffer", data.size());
		memcpy(stagingBuffer->GetMappedPointer(), data.data(), data.size());
		detail::g_stagingBatch.emplace_back(detail::StageDestination::eStageDstBuffer, stagingBuffer).dstBuffer = dst_buffer;
	}

	void Factory::EnqueueTextureUpload(Image* dst_image, const ByteArrayRef& data)
	{
		Buffer* stagingBuffer = CreateStagingBuffer("texture-staging-buffer", data.size());
		memcpy(stagingBuffer->GetMappedPointer(), data.data(), data.size());
		detail::g_stagingBatch.emplace_back(detail::StageDestination::eStageDstImage, stagingBuffer).dstImage = dst_image;
	}

	void Factory::FlushUploadBatch()
	{
		if (!detail::g_stagingBatch.empty())
		{
			Queue* queue = GetContext().GetDevice()->GetQueue(QueueType::Graphics);
			const AutoSubmitCommandBuffer autoSubmit { queue->AllocateCommandBuffer(), queue };
			CommandBuffer* commands = autoSubmit.GetCommandBuffer();
			commands->Begin();
			for (auto& stage : detail::g_stagingBatch)
			{
				if (stage.dstType == detail::StageDestination::eStageDstBuffer)
				{
					commands->CopyBuffer(stage.stagingBuffer.Get(), stage.dstBuffer);
				}
				else // stage.dstType == StagingDesc::eImage
				{
					commands->TransitionImage(stage.dstImage,
					                          ImageLayout::Undefined, PipelineStageFlagBits::TopOfPipe, AccessFlagBits::None,
					                          ImageLayout::TransferDstOptimal, PipelineStageFlagBits::Transfer, AccessFlagBits::TransferWrite);

					commands->CopyBufferToImage(stage.stagingBuffer.Get(), stage.dstImage, ImageLayout::TransferDstOptimal);

					commands->TransitionImage(stage.dstImage,
					                          ImageLayout::TransferDstOptimal, PipelineStageFlagBits::Transfer, AccessFlagBits::TransferWrite,
					                          ImageLayout::ShaderReadOnlyOptimal, PipelineStageFlagBits::FragmentShader, AccessFlagBits::ShaderRead);
				}

			}
			commands->End();
		}
		detail::g_stagingBatch.clear();
	}

	u32 Factory::GetFormatSize(Format format)
	{
		switch (format)
		{
		case Format::UNDEFINED:
			return 0;
		case Format::R8_UNORM:
		case Format::R8_SNORM:
		case Format::R8_UINT: 
		case Format::R8_SINT: 
		case Format::R8_SRGB:
			return 1;
		case Format::R8G8_UNORM:
		case Format::R8G8_SNORM:
		case Format::R8G8_UINT: 
		case Format::R8G8_SINT: 
		case Format::R8G8_SRGB:
			return 2;
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SNORM:
		case Format::R8G8B8_UINT: 
		case Format::R8G8B8_SINT: 
		case Format::R8G8B8_SRGB: 
		case Format::B8G8R8_UNORM:
		case Format::B8G8R8_SNORM:
		case Format::B8G8R8_UINT: 
		case Format::B8G8R8_SINT: 
		case Format::B8G8R8_SRGB:
			return 3;
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_SNORM:
		case Format::R8G8B8A8_UINT: 
		case Format::R8G8B8A8_SINT: 
		case Format::R8G8B8A8_SRGB: 
		case Format::B8G8R8A8_UNORM:
		case Format::B8G8R8A8_SNORM:
		case Format::B8G8R8A8_UINT: 
		case Format::B8G8R8A8_SINT: 
		case Format::B8G8R8A8_SRGB:
			return 4;
		case Format::R16_UNORM:
		case Format::R16_SNORM:
		case Format::R16_UINT: 
		case Format::R16_SINT: 
		case Format::R16_SFLOAT:
			return 2;
		case Format::R16G16_UNORM:
		case Format::R16G16_SNORM:
		case Format::R16G16_UINT: 
		case Format::R16G16_SINT: 
		case Format::R16G16_SFLOAT:
			return 4;
		case Format::R16G16B16_UNORM:
		case Format::R16G16B16_SNORM:
		case Format::R16G16B16_UINT: 
		case Format::R16G16B16_SINT: 
		case Format::R16G16B16_SFLOAT:
			return 6;
		case Format::R16G16B16A16_UNORM:
		case Format::R16G16B16A16_SNORM:
		case Format::R16G16B16A16_UINT: 
		case Format::R16G16B16A16_SINT: 
		case Format::R16G16B16A16_SFLOAT:
			return 8;
		case Format::R32_UINT: 
		case Format::R32_SINT: 
		case Format::R32_SFLOAT:
			return 4;
		case Format::R32G32_UINT:
		case Format::R32G32_SINT:
		case Format::R32G32_SFLOAT:
			return 8;
		case Format::R32G32B32_UINT:
		case Format::R32G32B32_SINT:
		case Format::R32G32B32_SFLOAT:
			return 12;
		case Format::R32G32B32A32_UINT:
		case Format::R32G32B32A32_SINT:
		case Format::R32G32B32A32_SFLOAT:
			return 16;
		case Format::D16_UNORM:
			return 2;
		case Format::D32_SFLOAT:
			return 4;
		case Format::S8_UINT: 
			return 1;
		case Format::D24_UNORM_S8_UINT:
			return 4;
		default: ;
		}
		return 0;
	}

	void Factory::BindSampledImage(ImageView* image_view)
	{
		GetContext().GetDevice()->BindSampledImage(image_view);
	}

	void Factory::BindStorageImage(ImageView* image_view)
	{
		GetContext().GetDevice()->BindStorageImage(image_view);
	}

	void Factory::BindUniformBuffer(Buffer* buffer, u64 offset, u64 size)
	{
		GetContext().GetDevice()->BindUniformBuffer(buffer, offset, size);
	}

	void Factory::BindStorageBuffer(Buffer* buffer, u64 offset, u64 size, u32 index)
	{
		GetContext().GetDevice()->BindStorageBuffer(buffer, offset, size, index);
	}

#define STATIC_ASSERT_VERTEX_FORMAT_OFFSET(ATTR) \
	static_assert(offsetof(VertexFormat, va##ATTR) == offsetof(VertexFormat, attributes[VertexAttribute::e##ATTR]))

FOREACH_VERTEX_FORMAT_ATTRIBUTES(STATIC_ASSERT_VERTEX_FORMAT_OFFSET, ;)

}
