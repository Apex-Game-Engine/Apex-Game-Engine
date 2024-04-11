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
		virtual ~Mesh() = default;
		virtual VertexBufferGPU getVertexBuffer() const = 0;
		virtual IndexBufferGPU getIndexBuffer() const = 0;
		virtual operator bool() const = 0;
	};

	struct StaticMesh : public Mesh
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

		void destroy(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
		{
			m_meshCPU = nullptr;

			m_vertexBuffer.destroy(device, pAllocator);
			m_indexBuffer.destroy(device, pAllocator);
		}

		operator bool() const
		{
			return m_vertexBuffer.m_count;
		}

		VertexBufferGPU getVertexBuffer() const override { return m_vertexBuffer; }
		IndexBufferGPU getIndexBuffer() const override { return m_indexBuffer; }
	};

	struct DynamicMesh : public Mesh
	{
		VertexBufferGPU m_vertexBuffer;
		IndexBufferGPU m_indexBuffer;

		void create(vk::VulkanDevice const& device, VertexInfo const& vertex_info, size_t count, IndexBufferCPU const& index_buffer_cpu, VkAllocationCallbacks const* pAllocator)
		{
			m_vertexBuffer.createMapped(device, vertex_info, count, pAllocator);
			m_indexBuffer.create(device, index_buffer_cpu, pAllocator);
		}

		void destroy(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
		{
			m_vertexBuffer.destroy(device, pAllocator);
			m_indexBuffer.destroy(device, pAllocator);
		}

		operator bool() const
		{
			return m_vertexBuffer.m_count;
		}

		VertexBufferGPU getVertexBuffer() const override { return m_vertexBuffer; }
		IndexBufferGPU getIndexBuffer() const override { return m_indexBuffer; }
	};

}
}
