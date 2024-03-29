﻿#include "Graphics/Vulkan/VulkanDebugMessenger.h"

#include "Core/Logging.h"
#include "Containers/AxArray.h"

#include <vulkan/vulkan.h>

#include "Graphics/Vulkan/VulkanFunctions.h"

namespace apex::vk {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	    void*                                            pUserData)
	{
		return static_cast<VulkanDebugMessenger*>(pUserData)->callback(messageSeverity, messageTypes, pCallbackData);
	}

	void VulkanDebugMessenger::create(VkInstance instance, VkAllocationCallbacks const* pAllocator)
	{
		const VkDebugUtilsMessengerCreateInfoEXT createInfo = getCreateInfo();

		axVerifyMsg(VK_SUCCESS == vk::CreateDebugUtilsMessengerEXT(instance, &createInfo, pAllocator, &debugMessenger),
			"Failed to create Vulkan debug utils messenger!"
		);
	}

	void VulkanDebugMessenger::destroy(VkInstance instance, VkAllocationCallbacks const* pAllocator)
	{
		vk::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, pAllocator);
	}
	
	VkDebugUtilsMessengerCreateInfoEXT VulkanDebugMessenger::getCreateInfo()
	{
		static VkDebugUtilsMessengerCreateInfoEXT ci {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = messageSeverityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = debug_messenger_callback,
			.pUserData = nullptr,
		};
		return ci;
	}

	VkBool32 VulkanDebugMessenger::callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			{
				axDebug(pCallbackData->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{
				axInfo(pCallbackData->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				axWarn(pCallbackData->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				axError(pCallbackData->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: break;
		}
		return VK_FALSE;
	}

}
