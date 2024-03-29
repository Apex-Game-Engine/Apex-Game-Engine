﻿#pragma once
#include <vulkan/vulkan_core.h>

namespace apex{
namespace vk {

	constexpr char vkCreateDebugUtilsMessengerEXT_Name[]  = "vkCreateDebugUtilsMessengerEXT";
	constexpr char vkDestroyDebugUtilsMessengerEXT_Name[] = "vkDestroyDebugUtilsMessengerEXT";
	constexpr char vkSetDebugUtilsObjectNameEXT_Name[]    = "vkSetDebugUtilsObjectNameEXT";

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

	VkResult SetDebugUtilsObjectNameEXT(
		VkDevice                                    device,
		const VkDebugUtilsObjectNameInfoEXT*        pNameInfo
	);
}
}
