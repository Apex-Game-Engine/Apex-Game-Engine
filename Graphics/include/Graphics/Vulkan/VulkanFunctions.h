#pragma once
#include <vulkan/vulkan_core.h>

namespace apex{
namespace vk {

	constexpr char VkCreateDebugUtilsMessengerEXT_Name[] = "vkCreateDebugUtilsMessengerEXT";
	constexpr char VkDestroyDebugUtilsMessengerEXT_Name[] = "VkDestroyDebugUtilsMessengerEXT";

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	    const VkAllocationCallbacks* pAllocator,
	    VkDebugUtilsMessengerEXT* pMessenger
	);

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator
	);
}
}
