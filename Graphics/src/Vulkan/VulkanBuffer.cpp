#include "Graphics/Vulkan/VulkanBuffer.h"

#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Vulkan/VulkanDevice.h"

namespace apex::vk {
	
	void VulkanBuffer::create(
		VulkanDevice const& device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharing_mode,
		AxArrayRef<uint32> const& queue_family_indices,
		VkMemoryPropertyFlags properties,
		VkAllocationCallbacks const* pAllocator)
	{
		VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage,
			.sharingMode = sharing_mode,
			.queueFamilyIndexCount = static_cast<uint32>(queue_family_indices.size),
			.pQueueFamilyIndices = queue_family_indices.data
		};

		axVerifyMsg(VK_SUCCESS == vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, pAllocator, &buffer), 
			"Failed to create vertex buffer!"
		);

		allocateMemory(device, properties, pAllocator);
	}

	void VulkanBuffer::destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyBuffer(device.logicalDevice, buffer, pAllocator);
		vkFreeMemory(device.logicalDevice, memory, pAllocator);
	}

	void VulkanBuffer::allocateMemory(VulkanDevice const& device, VkMemoryPropertyFlags properties, VkAllocationCallbacks const* pAllocator)
	{
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device.logicalDevice, buffer, &memoryRequirements);

		auto deviceMemoryType = device.findSuitableMemoryType(memoryRequirements.memoryTypeBits, properties);

		VkMemoryAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = deviceMemoryType
		};

		axVerifyMsg(VK_SUCCESS == vkAllocateMemory(device.logicalDevice, &allocateInfo, pAllocator, &memory),
			"Failed to allocate vertex buffer memory!"
		);

		vkBindBufferMemory(device.logicalDevice, buffer, memory, 0);
	}

	void VulkanBuffer::loadVertexBufferData(VulkanDevice const& device, gfx::VertexBufferCPU const& cpu_buffer)
	{
		void* data;
		axVerifyMsg(VK_SUCCESS == vkMapMemory(device.logicalDevice, memory, 0, cpu_buffer.sizeInBytes(), 0, &data),
			"Failed to map buffer memory!"
		);

		apex::memcpy_s<float>(data, cpu_buffer.size(), cpu_buffer.getData().data, cpu_buffer.size());

		vkUnmapMemory(device.logicalDevice, memory);
	}

	void VulkanBuffer::loadIndexBufferData(VulkanDevice const& device, gfx::IndexBufferCPU const& cpu_buffer)
	{
		void* data;
		axVerifyMsg(VK_SUCCESS == vkMapMemory(device.logicalDevice, memory, 0, cpu_buffer.sizeInBytes(), 0, &data),
			"Failed to map buffer memory!"
		);

		apex::memcpy_s<uint32>(data, cpu_buffer.size(), cpu_buffer.getData().data, cpu_buffer.size());

		vkUnmapMemory(device.logicalDevice, memory);
	}

	void VulkanBuffer::CreateVertexBufferGPU(
		VulkanBuffer& vertex_buffer,
		VulkanDevice const& device,
		gfx::VertexBufferCPU const& cpu_buffer,
		VkAllocationCallbacks const* pAllocator)
	{
		size_t bufferSize = cpu_buffer.sizeInBytes();

		// Create a temporary staging buffer
		VulkanBuffer stagingBuffer;

		VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = bufferSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &device.queueFamilyIndices.transferFamily.value()
		};

		axVerifyMsg(VK_SUCCESS == vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, pAllocator, &stagingBuffer.buffer),
			"Failed to create staging buffer!"
		);
		stagingBuffer.allocateMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pAllocator);

		// Fill the buffer
		stagingBuffer.loadVertexBufferData(device, cpu_buffer);

		// Create the vertex buffer
		vertex_buffer.create(
			device,
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			,, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, pAllocator
		);

		VkBufferCreateInfo vertexBufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = bufferSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &device.queueFamilyIndices.transferFamily.value()
		};

		// Copy vertex data from staging buffer to vertex buffer
		CopyBufferData(device, vertex_buffer, stagingBuffer, bufferSize);

		// Cleanup staging buffer
		stagingBuffer.destroy(device, pAllocator);
	}

	void VulkanBuffer::CreateIndexBufferGPU(
		VulkanBuffer& index_buffer,
		VulkanDevice const& device,
		gfx::IndexBufferCPU const& cpu_buffer,
		VkAllocationCallbacks const* pAllocator)
	{
		size_t bufferSize = cpu_buffer.sizeInBytes();

		// Create a temporary staging buffer
		VulkanBuffer stagingBuffer;
		stagingBuffer.create(
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			,, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pAllocator
		);

		// Fill the buffer
		stagingBuffer.loadIndexBufferData(device, cpu_buffer);

		// Create the index buffer
		index_buffer.create(
			device,
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			,, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, pAllocator
		);

		// Copy index data from staging buffer to index buffer
		CopyBufferData(device, index_buffer, stagingBuffer, bufferSize);

		// Cleanup staging buffer
		stagingBuffer.destroy(device, pAllocator);
	}

	void VulkanBuffer::CopyBufferData(VulkanDevice const& device, VulkanBuffer const& dst_buffer, VulkanBuffer const& src_buffer, VkDeviceSize size)
	{
		VkCommandBuffer transferCommandBuffer = device.beginOneShotCommandBuffer(device.transferCommandPool);

		VkBufferCopy copyRegion {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size
		};

		vkCmdCopyBuffer(transferCommandBuffer, src_buffer.buffer, dst_buffer.buffer, 1, &copyRegion);

		vkEndCommandBuffer(transferCommandBuffer);

		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			//.waitSemaphoreCount = ,
			//.pWaitSemaphores = ,
			//.pWaitDstStageMask = ,
			.commandBufferCount = 1,
			.pCommandBuffers = &transferCommandBuffer,
			// .signalSemaphoreCount = ,
			// .pSignalSemaphores = ,
		};

		vkQueueSubmit(device.transferQueue, 1, &submitInfo, VK_NULL_HANDLE);

		// Here, we're synchronously waiting on the queue to go idle
		// TODO: Better synchronize transfer operations
		//   1. Use Fences to fire multiple commands at once and wait for them before starting submitting
		//   2. Use Semaphores to schedule the rendering commands on the GPU to start after the transfer commands are completed - no CPU side waiting
		vkQueueWaitIdle(device.transferQueue);

		vkFreeCommandBuffers(device.logicalDevice, device.transferCommandPool, 1, &transferCommandBuffer);
	}
}
