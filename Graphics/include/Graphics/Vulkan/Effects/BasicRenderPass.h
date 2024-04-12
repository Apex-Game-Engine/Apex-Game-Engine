#pragma once
#include "Graphics/Vulkan/VulkanRenderPass.h"

namespace apex {
namespace vk {

	struct BasicRenderPass : public VulkanRenderPass
	{
		void create(
			VkDevice device,
			VkFormat swapchain_image_format,
			VulkanImage const* depth_image,
			VkAllocationCallbacks const* pAllocator) override;
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator) override;
	};

}
}
