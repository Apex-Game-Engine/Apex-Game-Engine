#pragma once

#include <vulkan/vulkan_core.h>

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
		VkBuffer       buffer{};
		VkDeviceMemory memory{};

		void create(
			VulkanDevice const& device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkAllocationCallbacks const* pAllocator);

		void createStagingBuffer(
			VulkanDevice const& device,
			VkDeviceSize size,
			VkAllocationCallbacks const* pAllocator);

		void createVertexBuffer(
			VulkanDevice const& device,
			gfx::VertexBufferCPU const& cpu_buffer,
			VkAllocationCallbacks const* pAllocator);

		void createIndexBuffer(
			VulkanDevice const& device,
			gfx::IndexBufferCPU const& cpu_buffer,
			VkAllocationCallbacks const* pAllocator);

		void destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);

		void allocateMemory(VulkanDevice const& device, VkMemoryPropertyFlags properties, VkAllocationCallbacks const* pAllocator);
		void loadVertexBufferData(VulkanDevice const& device, gfx::VertexBufferCPU const& cpu_buffer);
		void loadIndexBufferData(VulkanDevice const& device, gfx::IndexBufferCPU const& cpu_buffer);

		static void CreateVertexBufferGPU(
			VulkanBuffer& vertex_buffer,
			VulkanDevice const& device,
			gfx::VertexBufferCPU const& cpu_buffer,
			VkAllocationCallbacks const* pAllocator);

		static void CreateIndexBufferGPU(
			VulkanBuffer& index_buffer,
			VulkanDevice const& device,
			gfx::IndexBufferCPU const& cpu_buffer,
			VkAllocationCallbacks const* pAllocator);


		static void CopyBufferData(VulkanDevice const& device, VulkanBuffer const& dst_buffer, VulkanBuffer const& src_buffer, VkDeviceSize size);
	};
		
}
}
