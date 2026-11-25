#include "Graphics/Renderer.h"

#include "Graphics/Factory.h"

#if 0

namespace apex::gfx {

	FrameData::FrameData(Device* device)
		: m_currentSwapchainImage(nullptr), m_frameIndex(0), m_frameCount(device->GetMaxFrameCount()), m_presentSubmitted(false)
	{
		Queue* queue = device->GetQueue(QueueType::Graphics);
		const u32 count = m_frameCount;
		const Dim2D dimensions = device->GetSurfaceDim();

		const ImageCreateDesc imageDesc {
			.imageType		= ImageType::Image2D,
			.format			= Format::B8G8R8A8_SRGB,
			.usageFlags		= ImageUsageFlagBits::ColorAttachment | ImageUsageFlagBits::TransferSrc,
			.dimensions		= { dimensions.width, dimensions.height },
			.requiredFlags	= MemoryPropertyFlagBits::DeviceLocal,
			.ownerQueue		= QueueType::Graphics,
		};

		m_commandBuffers.SetSize(count);
		m_images.SetSize(count);

		const char* imageName[] = {
			"BackBuffer_0",
			"BackBuffer_1",
			"BackBuffer_2",
		};

		for (u32 i = 0; i < count; i++)
		{
			m_commandBuffers[i] = queue->AllocateCommandBuffer(i);
			m_images[i] = Factory::CreateImage(imageName[i], imageDesc);
		}

		m_presentSubmitted = true;
	}

	FrameData::~FrameData()
	{
		for (auto image : m_images)
		{
			delete image;
		}

		for (auto cmdbuf : m_commandBuffers)
		{
			delete cmdbuf;
		}
	}

	void FrameData::BeginFrame()
	{
		axAssertFmt(m_presentSubmitted, "Previous frame was not submitted! Did you forget to call FrameData::EndFrame?");
		m_frameIndex = (m_frameIndex + 1) % m_frameCount;

		Context& ctx = Factory::GetContext();
		const QueueType queueType = QueueType::Graphics;
		Queue* queue = ctx.GetDevice()->GetQueue(queueType);
		CommandBuffer* cmdbuf = GetCommandBuffer();
		Image* presentImage = GetBackBufferImage();
		m_currentSwapchainImage = ctx.GetDevice()->AcquireNextImage();
		m_presentSubmitted = false;
		
		axAssert(m_currentSwapchainImage);

		queue->ResetCommandBuffers(m_frameIndex);

		cmdbuf->Begin();

		cmdbuf->TransitionImage(presentImage,
		                        ImageLayout::Undefined,					PipelineStageFlagBits::TopOfPipe,				AccessFlagBits::None,
		                        ImageLayout::ColorAttachmentOptimal,	PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite);
	}

	void FrameData::EndFrame()
	{
		Context& ctx = Factory::GetContext();
		const QueueType queueType = QueueType::Graphics;
		Queue* queue = ctx.GetDevice()->GetQueue(queueType);
		CommandBuffer* cmdbuf = GetCommandBuffer();
		Image* presentImage = GetBackBufferImage();

		// 1. Transition layout from ColorAttachmentOptimal to TransferSrcOptimal
		// 2. Flush all ColorAttachmentWrite operations from the ColorAttachmentOutput stage of the previous command
		// 3. Until then, halt all TransferRead operations from the Transfer stage of the next command
		cmdbuf->TransitionImage(presentImage,
		                        ImageLayout::ColorAttachmentOptimal,	PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite,
		                        ImageLayout::TransferSrcOptimal,		PipelineStageFlagBits::Transfer,				AccessFlagBits::TransferRead);

		cmdbuf->TransitionImage(m_currentSwapchainImage,
		                        ImageLayout::Undefined,					PipelineStageFlagBits::Transfer,				AccessFlagBits::None,
		                        ImageLayout::TransferDstOptimal,		PipelineStageFlagBits::Transfer,				AccessFlagBits::TransferWrite);

		cmdbuf->CopyImage(presentImage, ImageLayout::TransferSrcOptimal, m_currentSwapchainImage, ImageLayout::TransferDstOptimal);

		cmdbuf->TransitionImage(m_currentSwapchainImage,
		                        ImageLayout::TransferDstOptimal,		PipelineStageFlagBits::Transfer,				AccessFlagBits::TransferWrite,
		                        ImageLayout::PresentSrcOptimal,			PipelineStageFlagBits::AllCommands,				AccessFlagBits::None);

		cmdbuf->End();

		queue->Submit(cmdbuf, true, gfx::PipelineStageFlagBits::Transfer, true);
		m_presentSubmitted = queue->Present();
	}

	void GBuffer::Init(Device* device, Dim2D dimensions)
	{
		const u32 count = device->GetMaxFrameCount();

		const char* albName[] = { "Gbuffer.Albedo_0", "Gbuffer.Albedo_1", "Gbuffer.Albedo_2" };
		const char* norName[] = { "Gbuffer.Normal_0", "Gbuffer.Normal_1", "Gbuffer.Normal_2" };
		const char* emiName[] = { "Gbuffer.Emissive_0", "Gbuffer.Emissive_1", "Gbuffer.Emissive_2" };
		const char* depName[] = { "Gbuffer.Depth_0", "Gbuffer.Depth_1", "Gbuffer.Depth_2" };

		const ImageCreateDesc albDesc {
			.imageType		= ImageType::Image2D,
			.format			= GetFormat(eAlbedo),
			.usageFlags		= ImageUsageFlagBits::ColorAttachment | ImageUsageFlagBits::Sampled | ImageUsageFlagBits::TransferSrc,
			.dimensions		= { dimensions.width, dimensions.height },
			.requiredFlags	= MemoryPropertyFlagBits::DeviceLocal,
			.ownerQueue		= QueueType::Graphics,
		};
		const ImageCreateDesc norDesc {
			.imageType		= ImageType::Image2D,
			.format			= GetFormat(eNormal),
			.usageFlags		= ImageUsageFlagBits::ColorAttachment | ImageUsageFlagBits::Sampled | ImageUsageFlagBits::TransferSrc,
			.dimensions		= { dimensions.width, dimensions.height },
			.requiredFlags	= MemoryPropertyFlagBits::DeviceLocal,
			.ownerQueue		= QueueType::Graphics,
		};
		const ImageCreateDesc emiDesc {
			.imageType		= ImageType::Image2D,
			.format			= GetFormat(eEmissive),
			.usageFlags		= ImageUsageFlagBits::ColorAttachment | ImageUsageFlagBits::Sampled | ImageUsageFlagBits::TransferSrc,
			.dimensions		= { dimensions.width, dimensions.height },
			.requiredFlags	= MemoryPropertyFlagBits::DeviceLocal,
			.ownerQueue		= QueueType::Graphics,
		};
		const ImageCreateDesc depDesc {
			.imageType		= ImageType::Image2D,
			.format			= GetFormat(eDepth),
			.usageFlags		= ImageUsageFlagBits::DepthStencilAttachment | ImageUsageFlagBits::Sampled,
			.dimensions		= { dimensions.width, dimensions.height },
			.requiredFlags	= MemoryPropertyFlagBits::DeviceLocal,
			.ownerQueue		= QueueType::Graphics,
		};

		m_images.SetSize(count * eTotalCount);

		for (u32 i = 0; i < count; i++)
		{
			m_images[i * eTotalCount + eAlbedo] = Factory::CreateImage(albName[i], albDesc);
			m_images[i * eTotalCount + eNormal] = Factory::CreateImage(norName[i], norDesc);
			m_images[i * eTotalCount + eEmissive] = Factory::CreateImage(emiName[i], emiDesc);
			m_images[i * eTotalCount + eDepth]  = Factory::CreateImage(depName[i], depDesc);
		}

		m_frameEnded = true;
	}

	void GBuffer::Shutdown()
	{
		for (auto image : m_images)
		{
			delete image;
		}
	}

	void GBuffer::Begin(CommandBuffer* cmdbuf)
	{
		axAssertFmt(m_frameEnded, "Previous GBuffer frame has not ended! Did you forget to call GBuffer::End?");
		m_frameIndex = (m_frameIndex + 1) % m_frameCount;

		const Image* albedoImage = GetImage(eAlbedo);
		const Image* normalImage = GetImage(eNormal);
		const Image* emissiveImage = GetImage(eEmissive);
		const Image* depthImage = GetImage(eDepth);

		const QueueType queueType = QueueType::Graphics;

		cmdbuf->TransitionImage(albedoImage,
		                        ImageLayout::Undefined,						PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite);

		cmdbuf->TransitionImage(normalImage,
		                        ImageLayout::Undefined,						PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite);

		cmdbuf->TransitionImage(emissiveImage,
		                        ImageLayout::Undefined,						PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite);

		constexpr PipelineStageFlags fragmentTestsEarlyLate = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests;
		constexpr AccessFlags depthStencilReadWrite = AccessFlagBits::DepthStencilAttachmentWrite | AccessFlagBits::DepthStencilAttachmentRead;
		cmdbuf->TransitionImage(depthImage,
		                        ImageLayout::Undefined,						PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead,
		                        ImageLayout::DepthStencilAttachmentOptimal, fragmentTestsEarlyLate,							depthStencilReadWrite);
	}

	void GBuffer::End(CommandBuffer* cmdbuf)
	{
		const Image* albedoImage = GetImage(eAlbedo);
		const Image* normalImage = GetImage(eNormal);
		const Image* emissiveImage = GetImage(eEmissive);
		const Image* depthImage = GetImage(eDepth);
		
		const QueueType queueType = QueueType::Graphics;

		cmdbuf->TransitionImage(albedoImage,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite,
		                        ImageLayout::ShaderReadOnlyOptimal,			PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead);

		cmdbuf->TransitionImage(normalImage,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite,
		                        ImageLayout::ShaderReadOnlyOptimal,			PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead);

		cmdbuf->TransitionImage(emissiveImage,
		                        ImageLayout::ColorAttachmentOptimal,		PipelineStageFlagBits::ColorAttachmentOutput,	AccessFlagBits::ColorAttachmentWrite,
		                        ImageLayout::ShaderReadOnlyOptimal,			PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead);

		constexpr PipelineStageFlags fragmentTestsEarlyLate = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests;
		constexpr AccessFlags depthStencilReadWrite = AccessFlagBits::DepthStencilAttachmentWrite | AccessFlagBits::DepthStencilAttachmentRead;
		cmdbuf->TransitionImage(depthImage,
		                        ImageLayout::DepthStencilAttachmentOptimal, fragmentTestsEarlyLate,							depthStencilReadWrite,
		                        ImageLayout::ShaderReadOnlyOptimal,			PipelineStageFlagBits::FragmentShader,			AccessFlagBits::ShaderRead);

		m_frameEnded = true;
	}

	Mesh* CreateMesh(VertexFormat vertex_format, u32 vertex_count, void* pVertices, u32 index_count, u32* pIndices)
	{
		const u32 stride = vertex_format.CalcVertexStride();
		const u32 vertexDataSize = vertex_count * stride;
		Buffer* vertexBuffer = Factory::CreateVertexBuffer("meshVB", vertexDataSize, nullptr);
		Buffer* indexBuffer = Factory::CreateIndexBuffer("meshIB", sizeof(u32) * index_count, nullptr);
		{
			BufferRef vertexStagingBuffer = Factory::CreateStagingBuffer("Staging-Vertex", vertexDataSize);
			BufferRef indexStagingBuffer = Factory::CreateStagingBuffer("Staging-Index", sizeof(u32) * index_count);

			apex::memcpy_s<float>(vertexStagingBuffer->GetMappedPointer(), vertexDataSize, pVertices, vertexDataSize);
			apex::memcpy_s<u32>(indexStagingBuffer->GetMappedPointer(), index_count, pIndices, index_count);

			{
				Queue* queue = Factory::GetContext().GetDevice()->GetQueue(QueueType::Graphics);
				AutoSubmitCommandBuffer autoSubmit { queue->AllocateCommandBuffer(), queue };
				CommandBuffer* commands = autoSubmit.GetCommandBuffer();
				commands->Begin();
				commands->CopyBuffer(vertexStagingBuffer.Get(), vertexBuffer);
				commands->CopyBuffer(indexStagingBuffer.Get(), indexBuffer);
				commands->End();
			}
		}

		return apex_new Mesh{ vertex_format, vertexBuffer, indexBuffer, vertex_count, index_count };
	}

	//void StaticMeshRenderer::CreateShaderPipelines(Device* device)
	//{
	//	const ShaderModuleRef vertexShader = device->CreateShaderModule("StaticMesh.Vertex", "X:\\ApexGameEngine-Vulkan\\build-msvc\\Apex\\Graphics\\shaders\\standard_phong.vert.spv");
	//	const ShaderModuleRef fragmentShader = device->CreateShaderModule("StaticMesh.Fragment", "X:\\ApexGameEngine-Vulkan\\build-msvc\\Apex\\Graphics\\shaders\\standard_phong.frag.spv");
	//	const GraphicsPipelineCreateDesc pipelineDesc {
	//		.shaderStages = {
	//			.vertexShader = vertexShader.Get(),
	//			.fragmentShader = fragmentShader.Get(),
	//		},
	//		.topology = PrimitiveTopology::TriangleList
	//	};
	//	m_geometryPipeline = device->CreateGraphicsPipeline("StaticMesh.Pipeline", pipelineDesc);
	//}

	//void StaticMeshRenderer::CreateRenderTargets(Device* device)
	//{
	//}

	//void StaticMeshRenderer::Cleanup(Device* device)
	//{
	//	delete m_geometryPipeline;
	//}

	//void StaticMeshRenderer::GenerateCommands(CommandBuffer* commands)
	//{

	//}
}
#endif