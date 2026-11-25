#pragma once

#include "Containers/AxArray.h"
#include "Core/Types.h"

namespace apex::plat
{
	class PlatformWindow;
}

namespace apex {
namespace gfx {
	// Forward Declarations
	enum class Api;
	class Context;
	class CommandBuffer;
	class Resource;

	struct DeviceProperties;
	struct DeviceFeatures;

	class Buffer;
	class Image;
	class BufferView;
	class ImageView;
	class DescriptorSet;

	class ShaderModule;
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;

	class Fence;
		
	constexpr u64 WHOLE_SIZE = u64(-1);
	constexpr u32 INVALID_INDEX = u32(-1);

	enum class MemoryPropertyFlagBits : u16
	{
		None            = 0x0000,
		DeviceLocal     = 0x0001,
		HostVisible     = 0x0002,
		HostCoherent    = 0x0004,
		HostCached      = 0x0008,
		LazilyAllocated = 0x0010,
	};
	using MemoryPropertyFlags = Flags<MemoryPropertyFlagBits>;
	DEFINE_BITWISE_OPERATORS(MemoryPropertyFlagBits, MemoryPropertyFlags);

	enum class MemoryAllocateFlagBits : u16
	{
		None                       = 0x0000,
		AllocateDedicated          = 0x0001,
		NeverAllocate              = 0x0002,
		CanAlias                   = 0x0004,
		HostAccessSequential       = 0x0010,
		HostAccessRandom           = 0x0020,
		HostAccessFallbackTransfer = 0x0040,
	};
	using MemoryAllocateFlags = Flags<MemoryAllocateFlagBits>;
	DEFINE_BITWISE_OPERATORS(MemoryAllocateFlagBits, MemoryAllocateFlags);

	enum class BufferUsageFlagBits : u16
	{
		Vertex      = 0x0001,
		Index       = 0x0002,
		Uniform     = 0x0004,
		Storage     = 0x0008,
		Indirect    = 0x0010,
		TransferSrc = 0x0020,
		TransferDst = 0x0040,
	};
	using BufferUsageFlags = Flags<BufferUsageFlagBits>;
	DEFINE_BITWISE_OPERATORS(BufferUsageFlagBits, BufferUsageFlags);

	enum class ImageUsageFlagBits : u16
	{
		TransferSrc = 0x00000001,
		TransferDst = 0x00000002,
		Sampled = 0x00000004,
		Storage = 0x00000008,
		ColorAttachment = 0x00000010,
		DepthStencilAttachment = 0x00000020,
		// TransientAttachment = 0x00000040,
		// InputAttachment = 0x00000080,
	};
	using ImageUsageFlags = Flags<ImageUsageFlagBits>;
	DEFINE_BITWISE_OPERATORS(ImageUsageFlagBits, ImageUsageFlags);

	enum class ResourceUsage : u32
	{
		TransferSrc,
		TransferDst,
		SampledImage,
		UniformBuffer = SampledImage,
		StorageImage,
		StorageBuffer = StorageImage,
		ColorAttachment,
		VertexBuffer = ColorAttachment,
		DepthStencilAttachment,
		IndexBuffer = DepthStencilAttachment,
		DepthStencilReadOnly,

		MAX_COUNT
	};

	enum class ImageLayout : u32
	{
		Undefined = 0,
	    General = 1,
	    ColorAttachmentOptimal = 2,
	    DepthStencilAttachmentOptimal = 3,
	    DepthStencilReadOnlyOptimal = 4,
	    ShaderReadOnlyOptimal = 5,
	    TransferSrcOptimal = 6,
	    TransferDstOptimal = 7,
		PresentSrcOptimal = 8,
	};

	enum class ImageType : u32
	{
		Image1D,
		Image2D,
		Image3D,
		ImageCube,
		Image1DArray,
		Image2DArray,
		ImageCubeArray,
	};

	enum class Format : u32
	{
		UNDEFINED,
		// 8-bit components
		// R
		R8_UNORM,
		R8_SNORM,
		R8_UINT,
		R8_SINT,
		R8_SRGB,
		// RG
		R8G8_UNORM,
		R8G8_SNORM,
		R8G8_UINT,
		R8G8_SINT,
		R8G8_SRGB,
		// RGB
		R8G8B8_UNORM,
		R8G8B8_SNORM,
		R8G8B8_UINT,
		R8G8B8_SINT,
		R8G8B8_SRGB,
		// BGR
		B8G8R8_UNORM,
		B8G8R8_SNORM,
		B8G8R8_UINT,
		B8G8R8_SINT,
		B8G8R8_SRGB,
		// RGBA
		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R8G8B8A8_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_SRGB,
		// BGRA
		B8G8R8A8_UNORM,
		B8G8R8A8_SNORM,
		B8G8R8A8_UINT,
		B8G8R8A8_SINT,
		B8G8R8A8_SRGB,
		// 16-bit components
		// R
		R16_UNORM,
		R16_SNORM,
		R16_UINT,
		R16_SINT,
		R16_SFLOAT,
		// RG
		R16G16_UNORM,
		R16G16_SNORM,
		R16G16_UINT,
		R16G16_SINT,
		R16G16_SFLOAT,
		// RGB
		R16G16B16_UNORM,
		R16G16B16_SNORM,
		R16G16B16_UINT,
		R16G16B16_SINT,
		R16G16B16_SFLOAT,
		// RGBA
		R16G16B16A16_UNORM,
		R16G16B16A16_SNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SINT,
		R16G16B16A16_SFLOAT,
		// 32-bit components
		// R
		R32_UINT,
		R32_SINT,
		R32_SFLOAT,
		// RG
		R32G32_UINT,
		R32G32_SINT,
		R32G32_SFLOAT,
		// RGB
		R32G32B32_UINT,
		R32G32B32_SINT,
		R32G32B32_SFLOAT,
		// RGBA
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32A32_SFLOAT,
		// Mixed components
		// BGR
		B10G11R11_UFLOAT,
		// Depth-Stencil formats
		D16_UNORM,
	    D32_SFLOAT,
	    S8_UINT,
	    D24_UNORM_S8_UINT,

		COUNT
	};

	enum class AccessFlagBits : u32
	{
	    None                        = 0,
		IndirectCommandRead         = 0x00000001,
	    IndexRead                   = 0x00000002,
	    VertexAttributeRead         = 0x00000004,
	    UniformRead                 = 0x00000008,
	    InputAttachmentRead         = 0x00000010,
	    ShaderRead                  = 0x00000020,
	    ShaderWrite                 = 0x00000040,
	    ColorAttachmentRead         = 0x00000080,
	    ColorAttachmentWrite        = 0x00000100,
	    DepthStencilAttachmentRead  = 0x00000200,
	    DepthStencilAttachmentWrite = 0x00000400,
	    TransferRead                = 0x00000800,
	    TransferWrite               = 0x00001000,
	    HostRead                    = 0x00002000,
	    HostWrite                   = 0x00004000,
	    MemoryRead                  = 0x00008000,
	    MemoryWrite                 = 0x00010000,
	};
	using AccessFlags = Flags<AccessFlagBits>;
	DEFINE_BITWISE_OPERATORS(AccessFlagBits, AccessFlags);

	enum class PipelineStageFlagBits : u32
	{
	    None                         = 0,
		TopOfPipe                    = 0x00000001,
	    DrawIndirect                 = 0x00000002,
	    VertexInput                  = 0x00000004,
	    VertexShader                 = 0x00000008,
	    TessellationControlShader    = 0x00000010,
	    TessellationEvaluationShader = 0x00000020,
	    GeometryShader               = 0x00000040,
	    FragmentShader               = 0x00000080,
	    EarlyFragmentTests           = 0x00000100,
	    LateFragmentTests            = 0x00000200,
	    ColorAttachmentOutput        = 0x00000400,
	    ComputeShader                = 0x00000800,
	    Transfer                     = 0x00001000,
	    BottomOfPipe                 = 0x00002000,
	    Host                         = 0x00004000,
	    AllGraphics                  = 0x00008000,
	    AllCommands                  = 0x00010000,
	};
	using PipelineStageFlags = Flags<PipelineStageFlagBits>;
	DEFINE_BITWISE_OPERATORS(PipelineStageFlagBits, PipelineStageFlags);

	enum class PrimitiveTopology : u8
	{
		PointLists,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		LineListWithAdjacency,
		LineStripWithAdjacency,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency,
		PatchList,
	};

	enum class LoadOp : u8
	{
		Load,
		Clear,
		DontCare,
	};

	enum class StoreOp : u8
	{
		Store,
		DontCare,
	};

	struct QueueType
	{
		enum Value {
			Graphics,
			Compute,
			Transfer,
			COUNT
		};

		Value value;

		constexpr QueueType(Value val) : value(val) {}
		explicit constexpr QueueType(u32 val) : value(static_cast<Value>(val)) {}

		constexpr operator Value() const { return value; }
		constexpr bool operator==(Value val) const { return value == val; }
	};

	struct GlobalDescriptorBinding
	{
		enum Value : u32
		{
			SampledImage,
			StorageImage,
			UniformBuffer,
			StorageBuffer,
			Sampler,

			COUNT
		} value;

		constexpr GlobalDescriptorBinding(Value val) : value(val) {}

		constexpr operator u32() const { return value; }
	};

	enum class DescriptorType : u8
	{
		eInputAttachment,
		eSampler,
		// eCombinedImageSampler,
		eSampledImage,
		eStorageImage,
		eUniformBuffer,
		eStorageBuffer,

		// HLSL descriptor types
		eShaderResourceView = eStorageImage,
		eConstantBufferView = eUniformBuffer,
		eUnorderedAccessView = eStorageBuffer,
		eTexture = eSampledImage,
	};

	#define VERT_ATTR_LOCATION_BITS 6
	#define VERT_ATTR_STREAM_BITS 6
	#define VERT_ATTR_OFFSET_BITS 12
	#define VERT_ATTR_FORMAT_BITS 8

	#define VERT_ATTR_LOCATION_MAX	((1 << VERT_ATTR_LOCATION_BITS)-1)
	#define VERT_ATTR_STREAM_MAX	((1 << VERT_ATTR_STREAM_BITS)-1)
	#define VERT_ATTR_OFFSET_MAX	((1 << VERT_ATTR_OFFSET_BITS)-1)
	#define VERT_ATTR_FORMAT_MAX	((1 << VERT_ATTR_FORMAT_BITS)-1)

#define FOREACH_VERTEX_FORMAT_ATTRIBUTES(MACRO, DELIMITER) \
	MACRO(POSITION) DELIMITER \
	MACRO(NORMAL) DELIMITER	\
	MACRO(TANGENT) DELIMITER \
	MACRO(BITANGENT) DELIMITER \
	MACRO(TEXCOORD0) DELIMITER \
	MACRO(TEXCOORD1) DELIMITER \
	MACRO(TEXCOORD2) DELIMITER \
	MACRO(TEXCOORD3) DELIMITER \
	MACRO(BONEINDICES0) DELIMITER \
	MACRO(BONEINDICES1) DELIMITER \
	MACRO(BONEWEIGHTS0) DELIMITER \
	MACRO(BONEWEIGHTS1) DELIMITER \
	MACRO(COLOR) DELIMITER \
	MACRO(USERVALUE0) DELIMITER \
	MACRO(USERVALUE1) DELIMITER \
	MACRO(USERVALUE2) DELIMITER \
	MACRO(USERVALUE3) DELIMITER

	struct VertexAttribute
	{
#define COMMA ,
#define DEFINE_VERTEX_ATTRIBUTE_ENUM(ATTR) e##ATTR

		enum Type : u32 {
			FOREACH_VERTEX_FORMAT_ATTRIBUTES(DEFINE_VERTEX_ATTRIBUTE_ENUM, COMMA)
			eMAXCOUNT,
		};

		u32 location : VERT_ATTR_LOCATION_BITS; // attribute location in shader
		u32 stream   : VERT_ATTR_STREAM_BITS;   // index of attribute stream
		u32 offset   : VERT_ATTR_OFFSET_BITS;   // offset within attribute stream
		u32 format   : VERT_ATTR_FORMAT_BITS;   // gfx::Format of attribute

		bool IsValid() const { return format != static_cast<u32>(Format::UNDEFINED); }
	};
	static_assert(sizeof(VertexAttribute) == sizeof(u32));

	struct VertexAttributeStream
	{
		u32 offset;
		u32 stride;
	};

	struct VertexFormat
	{
		
#define DEFINE_VERTEX_ATTRIBUTE_MEMBER(ATTR) VertexAttribute va##ATTR

		union {
			struct {
				FOREACH_VERTEX_FORMAT_ATTRIBUTES(DEFINE_VERTEX_ATTRIBUTE_MEMBER, ;);
			};
			VertexAttribute attributes[VertexAttribute::eMAXCOUNT];
		};

		[[nodiscard]] u64 CalcHash() const;

		[[nodiscard]] bool HasAttribute(VertexAttribute::Type attribute) const
		{
			return attributes[attribute].IsValid();
		}

		bool Validate() const;
		u32 CalcVertexStride() const;
		u32 CalcStreamCount() const;
		void RetrieveStreams(AxSpan<VertexAttributeStream> streams) const;
		bool RetrieveStreamAtIndex(u32 stream_index, VertexAttributeStream& out_stream) const;
		u32 CalcAttributeCount() const;
		const char* ToString(char* const buf, u32 bufsz);
	};

	struct CommandBufferCreateDesc;

	struct BufferCreateDesc
	{
		size_t										size;
		BufferUsageFlags							usageFlags;
		MemoryPropertyFlags							requiredFlags;
		MemoryPropertyFlags							preferredFlags;
		MemoryAllocateFlags							memoryFlags;
		QueueType									ownerQueue;
		bool										createMapped;
		u32											alignment;
		const void*									pInitialData;
	};

	struct ImageRegion
	{
		Dim3D										offset;
		Dim3D										extent;
	};

	struct ImageCreateDesc
	{
		ImageType									imageType;
		Format										format;
		ImageUsageFlags								usageFlags;
		Dim3D										dimensions;
		MemoryPropertyFlags							requiredFlags;
		MemoryPropertyFlags							preferredFlags;
		MemoryAllocateFlags							memoryFlags;
		QueueType									ownerQueue;
		bool										createMapped;
		const void*									pInitialData;
	};

	struct ImageViewCreateDesc
	{
		Image*										image{};
		ImageRegion									region;
	};

	struct GraphicsPipelineShaderStagesDesc
	{
		ShaderModule*								vertexShader;
		ShaderModule*								fragmentShader;
		// TODO: Add other shader stages as required
	};

	struct BlendState {};

	struct GraphicsPipelineAttachmentDesc
	{
		Format										format;
		BlendState									blendState;
	};

	struct DepthTestDesc
	{
		bool enableDepthTest;
		bool enableDepthWrite;
		// depthCompareOp
	};

	struct GraphicsPipelineCreateDesc
	{
		GraphicsPipelineShaderStagesDesc			shaderStages;
		PrimitiveTopology							topology;
		VertexFormat								vertexFormat;
		AxSpan<GraphicsPipelineAttachmentDesc>	colorAttachments;
		Format										depthAttachmentFormat;
		DepthTestDesc								depthTest;
	};

	struct ComputePipelineCreateDesc
	{
		ShaderModule*								computeShader;
	};

	struct QueueSubmitDesc
	{
		AxSpan<CommandBuffer*>					commandBuffers;
		Fence*										fence;
		PipelineStageFlags							fenceWaitStageMask;
		u64											fenceWaitValue;
		u64											fenceSignalValue;
		bool										waitImageAcquired;
		PipelineStageFlags							imageAcquiredWaitStageMask;
		bool										signalRenderComplete;
	};

	/// Graphics Factory Functions

	struct Factory
	{
		static void InitContext(Api api, plat::PlatformWindow& window);
		static void ShutdownContext();
		static Context& GetContext();

		static GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc);
		static ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc);

		static Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc);
		static Buffer* CreateVertexBuffer(const char* name, size_t size, const void* initial_data);
		static Buffer* CreateVertexBuffer(const char* name, size_t size, ByteArrayRef const& initial_data);
		static Buffer* CreateIndexBuffer(const char* name, size_t size, const void* initial_data);
		static Buffer* CreateIndexBuffer(const char* name, size_t size, ByteArrayRef const& initial_data);
		static Buffer* CreateStagingBuffer(const char* name, size_t size);
		static Buffer* CreateUniformBuffer(const char* name, size_t size);
		static Buffer* CreateStorageBuffer(const char* name, size_t size);

		static Image* CreateImage(const char* name, ImageCreateDesc const& desc);
		static Image* CreateDepthImage(const char* name, Dim2D dimensions, Format format = Format::D32_SFLOAT);
		static Image* CreateTexture2D(const char* name, Dim2D dimensions, Format format = Format::R8G8B8A8_UNORM);

		static ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc);

		static void CopyBufferToBuffer(Buffer* dst_buffer, Buffer* src_buffer);
		static void CopyBufferToImage(Image* dst_image, Buffer* src_buffer);

		static void EnqueueBufferUpload(Buffer* dst_buffer, const ByteArrayRef& data);
		static void EnqueueTextureUpload(Image* dst_image, const ByteArrayRef& data);
		static void FlushUploadBatch();

		static u32 GetFormatSize(Format format);

		static void BindSampledImage(ImageView* image_view);
		static void BindStorageImage(ImageView* image);
		static void BindUniformBuffer(Buffer* buffer, u64 offset = 0, u64 size = WHOLE_SIZE);
		static void BindStorageBuffer(Buffer* buffer, u64 offset = 0, u64 size = WHOLE_SIZE, u32 index = INVALID_INDEX);
	};

} // namespace gfx
} // namespace apex
