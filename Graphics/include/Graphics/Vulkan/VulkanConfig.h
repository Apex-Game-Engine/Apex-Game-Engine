#pragma once

#include <vulkan/vulkan_core.h>

namespace apex {
namespace vk {

	struct VulkanConfig
	{
	#ifdef APEX_VK_ENABLE_VALIDATION
		inline static bool kEnableDebugLayers = true;
		inline static const char* kValidationLayerNames[] = {
			"VK_LAYER_KHRONOS_validation",
		};
	#else
		inline static const bool kEnableDebugLayers = false;
		inline static const char* kValidationLayerNames = nullptr;
	#endif

		inline static const char* kRequiredInstanceExtensions[] = {
			// Required extensions for window surface creation
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

			// TODO: Add other required extensions here

			// This must be the last extension
	#ifdef APEX_VK_ENABLE_VALIDATION
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	#endif
		};

		inline static const char* kRequiredDeviceExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};


}
}
