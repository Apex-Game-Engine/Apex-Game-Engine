#pragma once
#include "vulkan/vulkan_core.h"

namespace apex {
namespace vk {
	struct VulkanSwapchainSupportDetails;

	bool check_validation_layer_support(const char* validationLayerNames[], size_t validationLayerCount);

	bool check_instance_extensions_support(const char* instanceExtensionNames[], size_t instanceExtensionCount);

	bool check_device_extensions_support(const char* deviceExtensionNames[], size_t deviceExtensionCount);

	auto query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface) -> VulkanSwapchainSupportDetails;

	void transition_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

}
}
