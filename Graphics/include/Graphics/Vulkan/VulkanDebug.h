#pragma once

#include <vulkan/vulkan_core.h>

namespace apex {
namespace vk {

	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info();

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	    void*                                            pUserData
	);

}
}
