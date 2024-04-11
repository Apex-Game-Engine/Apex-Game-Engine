#include "Graphics/Geometry/VertexBufferGPU.h"

namespace apex::gfx {

	void VertexBufferGPU::createMapped(vk::VulkanDevice const& device, VertexInfo const& vertex_info, size_t count, VkAllocationCallbacks const* pAllocator)
	{
		m_vertexInfo = vertex_info;
		m_count = count;

		size_t bufferSize = vertex_info.stride * count;

		// Create the vertex buffer
		m_buffer.createVertexBuffer(device, bufferSize, true, pAllocator);
	}

	void VertexBufferGPU::create(vk::VulkanDevice const& device, VertexBufferCPU const& vertex_buffer_cpu, VkAllocationCallbacks const* pAllocator)
	{
		m_vertexInfo = vertex_buffer_cpu.vertexInfo();
		m_count = vertex_buffer_cpu.count();

		size_t bufferSize = vertex_buffer_cpu.sizeInBytes();

		// Create a temporary staging buffer
		vk::VulkanBuffer stagingBuffer;
		stagingBuffer.createStagingBuffer(device, bufferSize, pAllocator);

		// Fill the buffer
		stagingBuffer.loadVertexBufferData(device, vertex_buffer_cpu);

		// Create the vertex buffer
		m_buffer.createVertexBuffer(device, bufferSize, false, pAllocator);

		// Copy vertex data from staging buffer to vertex buffer
		vk::VulkanBuffer::CopyBufferData(device, m_buffer, stagingBuffer, bufferSize);

		// Cleanup staging buffer
		stagingBuffer.destroy(device, pAllocator);
	}


	void VertexBufferGPU::destroy(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		m_buffer.destroy(device, pAllocator);
	}
}
