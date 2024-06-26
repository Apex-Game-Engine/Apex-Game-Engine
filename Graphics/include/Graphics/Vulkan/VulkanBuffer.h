﻿#pragma once

#include <vulkan/vulkan_core.h>

#include <vma.h>
#include "VulkanDevice.h"

namespace apex::gfx
{
	class IndexBufferCPU;
	class VertexBufferCPU;
}

namespace apex {
namespace vk {

	struct VulkanBuffer
	{
		VkBuffer          buffer{};
		VmaAllocation     allocation{};
		VmaAllocationInfo allocation_info{};

		void create(
			VulkanDevice const& device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkSharingMode sharing_mode,
			AxArrayRef<uint32> const& queue_family_indices,
			VmaMemoryUsage memory_usage,
			VmaAllocationCreateFlags vma_flags,
			VkAllocationCallbacks const* pAllocator);

		void createStagingBuffer(
			VulkanDevice const& device,
			VkDeviceSize size,
			VkAllocationCallbacks const* pAllocator);

		void createVertexBuffer(
			VulkanDevice const& device,
			VkDeviceSize size,
			bool mapped, VkAllocationCallbacks const* pAllocator);

		void createIndexBuffer(
			VulkanDevice const& device,
			VkDeviceSize size,
			VkAllocationCallbacks const* pAllocator);

		void destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);

		void loadVertexBufferData(VulkanDevice const& device, gfx::VertexBufferCPU const& cpu_buffer);
		void loadIndexBufferData(VulkanDevice const& device, gfx::IndexBufferCPU const& cpu_buffer);
		void* getMappedMemory() const;
		
		void map(VulkanDevice const& device, void** ppData) const;
		void unmap(VulkanDevice const& device) const;

		static void CopyBufferData(VulkanDevice const& device, VulkanBuffer const& dst_buffer, VulkanBuffer const& src_buffer, VkDeviceSize size);

		[[nodiscard]] auto size() const -> size_t;
	};
		
}
}
