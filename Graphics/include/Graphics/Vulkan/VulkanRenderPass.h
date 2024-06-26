﻿#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanImage.h"

namespace apex {
namespace vk {
		
	struct VulkanRenderPass
	{
		VkRenderPass renderPass{};

		virtual ~VulkanRenderPass() = default;
		virtual void create(
			VkDevice device,
			VkFormat swapchain_image_format,
			VulkanImage const* depth_image,
			VkAllocationCallbacks const* pAllocator) = 0;
		virtual void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator) = 0;
	};

}
}
