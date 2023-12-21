#include "Graphics/Geometry/IndexBufferGPU.h"

namespace apex::gfx {

	void IndexBufferGPU::create(vk::VulkanDevice const& device, IndexBufferCPU const& index_buffer_cpu, VkAllocationCallbacks const* pAllocator)
	{
		m_count = index_buffer_cpu.count();

		size_t bufferSize = index_buffer_cpu.sizeInBytes();

		// Create a temporary staging buffer
		vk::VulkanBuffer stagingBuffer;
		stagingBuffer.createStagingBuffer(device, bufferSize, pAllocator);

		// Fill the buffer
		stagingBuffer.loadIndexBufferData(device, index_buffer_cpu);

		// Create the index buffer
		m_buffer.createIndexBuffer(device, bufferSize, pAllocator);

		// Copy index data from staging buffer to index buffer
		vk::VulkanBuffer::CopyBufferData(device, m_buffer, stagingBuffer, bufferSize);

		// Cleanup staging buffer
		stagingBuffer.destroy(device.logicalDevice, pAllocator);
	}

	void IndexBufferGPU::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		m_buffer.destroy(device, pAllocator);
	}
}
