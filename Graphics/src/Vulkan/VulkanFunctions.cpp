#include "Graphics/Vulkan/VulkanFunctions.h"

#include "Core/Asserts.h"

namespace apex {

	VkResult vk::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger)
	{
		static auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, VkCreateDebugUtilsMessengerEXT_Name);

		if (nullptr != func)
		{
			return func(instance, pCreateInfo, pAllocator, pMessenger);
		}

		axAssertMsg(false, "Vulkan extension not found: VkCreateDebugUtilsMessengerEXT");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void vk::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator)
	{
		static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, VkDestroyDebugUtilsMessengerEXT_Name);

		if (nullptr != func)
		{
			return func(instance, messenger, pAllocator);
		}
		axAssertMsg(false, "Vulkan extension not found: VkDestroyDebugUtilsMessengerEXT");
	}
}
