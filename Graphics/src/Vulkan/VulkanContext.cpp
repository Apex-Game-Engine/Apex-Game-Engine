#include "Graphics/Vulkan/VulkanContext.h"

#include "Core/Asserts.h"
#include "Core/Logging.h"
#include "Graphics/Vulkan/VulkanFunctions.h"

namespace apex {
namespace gfx {

namespace detail {

	extern constexpr VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info();
	extern VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_messenger_callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	    void*                                            pUserData
	);
	extern bool check_validation_layer_support();


#ifdef APEX_VK_ENABLE_VALIDATION
	bool kEnableDebugLayers = true;
	const char* kValidationLayerNames[] = {
		"VK_LAYER_KHRONOS_validation",
	};
#else
	const bool kEnableDebugLayers = false;
	const char* kValidationLayerNames[] = {
	};
#endif

	const char* kRequiredInstanceExtensions[] = {
		// Required extensions for window surface creation
		"VK_KHR_surface",
		"VK_KHR_win32_surface",

		// TODO: Add other required extensions here

		// This must be the last extension
#ifdef APEX_VK_ENABLE_VALIDATION
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	const char* kRequiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

} // namespace detail

	VulkanContext* VulkanContext::s_pInstance = nullptr;

	void VulkanContext::initialize(const char* app_name, Window* pwindow, bool enable_debugging)
	{
		s_pInstance = this;

		_initializeVulkan(app_name, pwindow, enable_debugging);
	}

	void VulkanContext::_initializeVulkan(const char* app_name, Window* pwindow, bool enable_debugging)
	{
		m_pWindow = pwindow;

		#ifdef APEX_VK_ENABLE_VALIDATION
		detail::kEnableDebugLayers = enable_debugging;
		#endif

		// Create a Vulkan instance
		_createInstance(app_name);

		// Add debug messenger for handling debug callbacks
		_createDebugMessenger();


	}

	void VulkanContext::_cleanupVulkan()
	{
	}

	void VulkanContext::_createInstance(const char* app_name)
	{
		// TODO: Extract application and engine version info from env macros
		VkApplicationInfo applicationInfo{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName   = app_name,
		.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
		.pEngineName        = "Apex Game Engine",
		.engineVersion      = VK_MAKE_VERSION(0, 2, 0),
		.apiVersion         = VK_API_VERSION_1_3,
		};

		VkInstanceCreateInfo instanceCreateInfo{
		.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		};

		// Check if validation layers are available
		axAssertMsg(!(detail::kEnableDebugLayers && !detail::check_validation_layer_support()),
			"Validation layers are requested but not supported!"
		);

		const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = detail::debug_messenger_create_info();

		if (detail::kEnableDebugLayers)
		{
			instanceCreateInfo.enabledLayerCount   = static_cast<uint32>(std::size(detail::kValidationLayerNames));
			instanceCreateInfo.ppEnabledLayerNames = detail::kValidationLayerNames;
			instanceCreateInfo.pNext = &debugMessengerCreateInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
			instanceCreateInfo.ppEnabledLayerNames = nullptr;
		}

		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32>(std::size(detail::kRequiredInstanceExtensions));
		instanceCreateInfo.ppEnabledExtensionNames = detail::kRequiredInstanceExtensions;

		axVerifyMsg(VK_SUCCESS == vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance),
			"Failed to create Vulkan instance!"
		);
	}

	void VulkanContext::_createDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = detail::debug_messenger_create_info();

		axAssertMsg(VK_SUCCESS == vk::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger),
			"Failed to create Vulkan debug utils messenger!"
		);
	}

} // namespace gfx
} // namespace apex
