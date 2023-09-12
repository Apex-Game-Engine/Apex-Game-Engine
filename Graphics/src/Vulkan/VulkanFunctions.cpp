#include "Graphics/Vulkan/VulkanFunctions.h"

#include "Core/Asserts.h"

namespace apex {

	VkResult vk::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger)
	{
		static auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, vkCreateDebugUtilsMessengerEXT_Name);

		if (nullptr != func)
		{
			return func(instance, pCreateInfo, pAllocator, pMessenger);
		}

		axAssertMsg(false, "Vulkan extension not found: vkCreateDebugUtilsMessengerEXT");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void vk::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator)
	{
		static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, vkDestroyDebugUtilsMessengerEXT_Name);

		if (nullptr != func)
		{
			return func(instance, messenger, pAllocator);
		}
		axAssertMsg(false, "Vulkan extension not found: vkDestroyDebugUtilsMessengerEXT");
	}

	VkResult vk::SetDebugUtilsObjectNameEXT(
		VkDevice device,
		const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
	{
		static auto func = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetDeviceProcAddr(device, vkSetDebugUtilsObjectNameEXT_Name);

		if (nullptr != func)
		{
			return func(device, pNameInfo);
		}
		axAssertMsg(false, "Vulkan extension not found: vkSetDebugUtilsObjectNameEXT");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
