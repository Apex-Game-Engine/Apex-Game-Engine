#include "Graphics/DebugRenderer.h"

#include "Graphics/GraphicsContext.h"
#include "Math/Vector4.h"
#include "Memory/AxHandle.h"
#include "Memory/MemoryManager.h"


namespace apex::gfx {

	DebugRenderer* DebugRenderer::Create(Device* device, DebugRendererCreateDesc const& desc)
	{
		ShaderModule* vertexShader = device->CreateShaderModule("DebugRenderer-VertexShader", R"(X:\ApexGameEngine-Vulkan\build-msvc\Apex\Graphics\spv\debug_lines.vert.spv)");
		ShaderModule* fragmentShader = device->CreateShaderModule("DebugRenderer-FragmentShader", R"(X:\ApexGameEngine-Vulkan\build-msvc\Apex\Graphics\spv\debug_lines.frag.spv)");;
		GraphicsPipeline* pipeline = device->CreateGraphicsPipeline("DebugRendererPipeline", 
			{
				.shaderStages = { vertexShader, fragmentShader },
				.topology = PrimitiveTopology::LineList,
			});
		delete vertexShader;
		delete fragmentShader;

		auto debugRenderer = apex_new(DebugRenderer)(device, pipeline);

		const BufferCreateDesc vertexBufferDesc {
			.size = desc.maxVertexCount * sizeof(math::Vector3),
			.usageFlags = BufferUsageFlagBits::Storage,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal | MemoryPropertyFlagBits::HostVisible | MemoryPropertyFlagBits::HostCoherent,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessRandom,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		const BufferCreateDesc indexBufferDesc {
			.size = desc.maxVertexCount * 2u * sizeof(math::Vector3),
			.usageFlags = BufferUsageFlagBits::Index,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal | MemoryPropertyFlagBits::HostVisible | MemoryPropertyFlagBits::HostCoherent,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessRandom,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		for (u32 i = 0; i < device->GetFramesInFlight(); i++)
		{
			debugRenderer->m_vertexBuffers[i] = device->CreateBuffer("DebugRenderer-VertexBuffer", vertexBufferDesc);
			debugRenderer->m_indexBuffers[i] = device->CreateBuffer("DebugRenderer-IndexBuffer", indexBufferDesc);

			device->BindStorageBuffer(debugRenderer->m_vertexBuffers[i]);
		}

		return debugRenderer;
	}

	DebugRenderer::~DebugRenderer()
	{
		for (u32 i = 0; i < 3; i++)
		{
			delete m_vertexBuffers[i];
			delete m_indexBuffers[i];
		}
		delete m_pipeline;
	}

	void DebugRenderer::NewFrame()
	{
		m_frameIndex = (m_frameIndex + 1) / m_device->GetFramesInFlight();
		m_vertexCount = 0;
		m_indexCount = 0;
	}

	void DebugRenderer::Draw(CommandBuffer* cmd) const
	{
		struct PushConstants
		{
			math::Vector4 color;
			u32 bufferIndex;
		} pc {
			.color = { 0.0, 1.0, 1.0, 1.0 },
			.bufferIndex = m_vertexBuffers[m_frameIndex]->GetBindlessIndex(BindlessDescriptorType::StorageBuffer)
		};

		ScopedGpuLabel(cmd);
		cmd->BindGraphicsPipeline(m_pipeline);
		cmd->PushConstants(pc);
		cmd->BindIndexBuffer(m_indexBuffers[m_frameIndex]);
		cmd->DrawIndexed(m_indexCount);
	}

	void DebugRenderer::Line(math::Vector3 const& v0, math::Vector3 const& v1)
	{
		const Buffer* vertexBuffer = m_vertexBuffers[m_frameIndex];
		const Buffer* indexBuffer = m_indexBuffers[m_frameIndex];

		axAssert((m_vertexCount + 2) * sizeof(math::Vector3) < vertexBuffer->GetSize());
		axAssert((m_indexCount + 2) * sizeof(u32) < indexBuffer->GetSize());

		math::Vector4* vertices = static_cast<math::Vector4*>(vertexBuffer->GetMappedPointer());
		u32* indices = static_cast<u32*>(indexBuffer->GetMappedPointer());

		const u32 startIndex = m_vertexCount;
		vertices[m_vertexCount++] = math::Vector4(v0, 1.0);
		vertices[m_vertexCount++] = math::Vector4(v1, 1.0);

		indices[m_indexCount++] = startIndex + 0;
		indices[m_indexCount++] = startIndex + 1;
	}

	void DebugRenderer::Cube(math::Vector3 const& pos, math::Vector3 const& size)
	{
		const Buffer* vertexBuffer = m_vertexBuffers[m_frameIndex];
		const Buffer* indexBuffer = m_indexBuffers[m_frameIndex];

		axAssert((m_vertexCount + 8) * sizeof(math::Vector4) < vertexBuffer->GetSize());
		axAssert((m_indexCount + 12) * sizeof(u32) < indexBuffer->GetSize());

		math::Vector4* vertices = static_cast<math::Vector4*>(vertexBuffer->GetMappedPointer());
		u32* indices = static_cast<u32*>(indexBuffer->GetMappedPointer());

		const math::Vector3 halfSize = size / 2.f;
		const u32 startIndex = m_vertexCount;
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{  halfSize.x,  halfSize.y,  halfSize.z }, 1.0); // right-top-front
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{  halfSize.x,  halfSize.y, -halfSize.z }, 1.0); // right-top-back
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{  halfSize.x, -halfSize.y, -halfSize.z }, 1.0); // right-bot-back
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{  halfSize.x, -halfSize.y,  halfSize.z }, 1.0); // right-bot-front
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{ -halfSize.x,  halfSize.y,  halfSize.z }, 1.0); // left-top-front
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{ -halfSize.x,  halfSize.y, -halfSize.z }, 1.0); // left-top-back
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{ -halfSize.x, -halfSize.y, -halfSize.z }, 1.0); // left-bot-back
		vertices[m_vertexCount++] = math::Vector4(pos + math::Vector3{ -halfSize.x, -halfSize.y,  halfSize.z }, 1.0); // left-bot-front

		// Right Face Edges
		indices[m_indexCount++] = startIndex + 0;
		indices[m_indexCount++] = startIndex + 1;
		indices[m_indexCount++] = startIndex + 1;
		indices[m_indexCount++] = startIndex + 2;
		indices[m_indexCount++] = startIndex + 2;
		indices[m_indexCount++] = startIndex + 3;
		indices[m_indexCount++] = startIndex + 3;
		indices[m_indexCount++] = startIndex + 0;
		// Left Face Edges
		indices[m_indexCount++] = startIndex + 4;
		indices[m_indexCount++] = startIndex + 5;
		indices[m_indexCount++] = startIndex + 5;
		indices[m_indexCount++] = startIndex + 6;
		indices[m_indexCount++] = startIndex + 6;
		indices[m_indexCount++] = startIndex + 7;
		indices[m_indexCount++] = startIndex + 7;
		indices[m_indexCount++] = startIndex + 4;
		// Connecting Edges
		indices[m_indexCount++] = startIndex + 0;
		indices[m_indexCount++] = startIndex + 4;
		indices[m_indexCount++] = startIndex + 1;
		indices[m_indexCount++] = startIndex + 5;
		indices[m_indexCount++] = startIndex + 2;
		indices[m_indexCount++] = startIndex + 6;
		indices[m_indexCount++] = startIndex + 3;
		indices[m_indexCount++] = startIndex + 7;
	}
}
