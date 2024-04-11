#include "Graphics/Vulkan/VulkanBuffer.h"

#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Vulkan/VulkanDevice.h"
#include "Graphics/Vulkan/VulkanFunctions.h"

namespace apex::vk {
	
	void VulkanBuffer::create(
		VulkanDevice const& device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharing_mode,
		AxArrayRef<uint32> const& queue_family_indices,
		VmaMemoryUsage memory_usage,
		VmaAllocationCreateFlags vma_flags,
		VkAllocationCallbacks const* pAllocator)
	{
		VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage,
			.sharingMode = sharing_mode,
			.queueFamilyIndexCount = static_cast<uint32>(queue_family_indices.count),
			.pQueueFamilyIndices = queue_family_indices.data
		};

		// allocateMemory(device, properties, pAllocator);
		VmaAllocationCreateInfo allocInfo {
			.flags = vma_flags,
			.usage = memory_usage,
		};

		axVerifyMsg(VK_SUCCESS == vmaCreateBuffer(device.m_allocator, &bufferCreateInfo, &allocInfo, &buffer, &allocation, &allocation_info),
			"Failed to create buffer!"
		);
	}

	void VulkanBuffer::createStagingBuffer(VulkanDevice const& device, VkDeviceSize size, VkAllocationCallbacks const* pAllocator)
	{
		uint32 queueFamilyIndices[] = { device.queueFamilyIndices.transferFamily.value() };

		create(
			device,
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			{ .data = queueFamilyIndices, .count = std::size(queueFamilyIndices) },
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			pAllocator);
	}
 
	void VulkanBuffer::createVertexBuffer(VulkanDevice const& device, VkDeviceSize size, bool mapped, VkAllocationCallbacks const* pAllocator)
	{
		uint32 queueFamilyIndices[] = { device.queueFamilyIndices.graphicsFamily.value(), device.queueFamilyIndices.transferFamily.value() };

		create(
			device,
			size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, // TODO: create custom flags to select these for each buffer
			VK_SHARING_MODE_CONCURRENT,
			{ .data = queueFamilyIndices, .count = std::size(queueFamilyIndices) },
			mapped ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE : VMA_MEMORY_USAGE_GPU_ONLY,
			mapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT : 0,
			pAllocator);

		VkDebugUtilsObjectNameInfoEXT objectNameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = VK_OBJECT_TYPE_BUFFER,
			.objectHandle = reinterpret_cast<uint64>(buffer),
			.pObjectName = "Vertex Buffer",
		};

		vk::SetDebugUtilsObjectNameEXT(device.logicalDevice, &objectNameInfo);
	}

	void VulkanBuffer::createIndexBuffer(VulkanDevice const& device, VkDeviceSize size, VkAllocationCallbacks const* pAllocator)
	{
		uint32 queueFamilyIndices[] = { device.queueFamilyIndices.graphicsFamily.value(), device.queueFamilyIndices.transferFamily.value() };

		create(
			device,
			size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_SHARING_MODE_CONCURRENT,
			{ .data = queueFamilyIndices, .count = std::size(queueFamilyIndices) },
			VMA_MEMORY_USAGE_GPU_ONLY,
			0, pAllocator);
	}

	void VulkanBuffer::destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		vmaDestroyBuffer(device.m_allocator, buffer, allocation);
	}

	void VulkanBuffer::loadVertexBufferData(VulkanDevice const& device, gfx::VertexBufferCPU const& cpu_buffer)
	{
		void* data;
		axVerifyMsg(VK_SUCCESS == vmaMapMemory(device.m_allocator, allocation, &data),
			"Failed to map buffer memory!"
		);

		apex::memcpy_s<float>(data, cpu_buffer.size(), cpu_buffer.data(), cpu_buffer.size());

		vmaUnmapMemory(device.m_allocator, allocation);
	}

	void VulkanBuffer::loadIndexBufferData(VulkanDevice const& device, gfx::IndexBufferCPU const& cpu_buffer)
	{
		void* data;
		axVerifyMsg(VK_SUCCESS == vmaMapMemory(device.m_allocator, allocation, &data),
			"Failed to map buffer memory!"
		);

		apex::memcpy_s<uint32>(data, cpu_buffer.count(), cpu_buffer.data().data, cpu_buffer.count());

		vmaUnmapMemory(device.m_allocator, allocation);
	}

	void* VulkanBuffer::getMappedMemory() const
	{
		axAssertMsg(allocation_info.pMappedData, "Buffer is not mapped!");
		return allocation_info.pMappedData;
	}

	void VulkanBuffer::map(VulkanDevice const& device, void **ppData) const
	{
		// TODO: Handle non-HOST_VISIBLE memory properly
		axVerifyMsg(VK_SUCCESS == vmaMapMemory(device.m_allocator, allocation, ppData),
			"Failed to map buffer memory! [TODO: Handle non-HOST_VISIBLE memory properly]"
		);
	}

	void VulkanBuffer::unmap(VulkanDevice const& device) const
	{
		// TODO: Handle non-HOST_COHERENT memory
		vmaUnmapMemory(device.m_allocator, allocation);
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

	size_t VulkanBuffer::size() const
	{
		return allocation_info.size;
	}
}
