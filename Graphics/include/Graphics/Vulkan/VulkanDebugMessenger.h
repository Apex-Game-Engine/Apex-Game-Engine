#pragma once

#include <vulkan/vulkan_core.h>

namespace apex {
namespace vk {

	struct VulkanDebugMessenger
	{
		void create(VkInstance instance, VkAllocationCallbacks const* pAllocator);
		void destroy(VkInstance instance, VkAllocationCallbacks const* pAllocator);

		VkDebugUtilsMessengerCreateInfoEXT getCreateInfo();

		VkBool32 callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

		VkDebugUtilsMessageSeverityFlagsEXT messageSeverityFlags =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		VkDebugUtilsMessageTypeFlagsEXT     messageTypeFlags =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		VkDebugUtilsMessengerEXT debugMessenger{};
	};

}
}
