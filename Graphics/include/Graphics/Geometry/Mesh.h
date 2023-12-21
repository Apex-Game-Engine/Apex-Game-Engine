#pragma once
#include "IndexBufferCPU.h"
#include "IndexBufferGPU.h"
#include "VertexBufferCPU.h"
#include "VertexBufferGPU.h"
#include "Graphics/Vulkan/VulkanBuffer.h"

namespace apex {
namespace gfx {
	class IndexBufferCPU;
	class VertexBufferCPU;

	struct MeshCPU
	{
		VertexBufferCPU m_vertexBufferCPU;
		IndexBufferCPU m_indexBufferCPU;
	};

	struct Mesh
	{
		MeshCPU const* m_meshCPU;

		VertexBufferGPU m_vertexBuffer;
		IndexBufferGPU m_indexBuffer;

		void create(vk::VulkanDevice const& device, MeshCPU const* mesh_cpu, VkAllocationCallbacks const* pAllocator)
		{
			m_meshCPU = mesh_cpu;

			m_vertexBuffer.create(device, m_meshCPU->m_vertexBufferCPU, pAllocator);
			m_indexBuffer.create(device, m_meshCPU->m_indexBufferCPU, pAllocator);
		}

		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
		{
			m_meshCPU = nullptr;

			m_vertexBuffer.destroy(device, pAllocator);
			m_indexBuffer.destroy(device, pAllocator);
		}

		operator bool() const
		{
			return m_vertexBuffer.m_count;
		}
	};

}
}
