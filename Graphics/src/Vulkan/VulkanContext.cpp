#include "Graphics/Vulkan/VulkanContext.h"

#include "Core/Asserts.h"
#include "Core/Logging.h"

namespace apex {
namespace gfx {

namespace detail {

	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info();
	bool check_validation_layer_support();
	

	static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_messenger_callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	    void*                                            pUserData
	);


#ifdef APEX_VK_ENABLE_VALIDATION
	bool s_enableDebugLayers = true;
	const char* s_validationLayerNames[] = {
		"VK_LAYER_KHRONOS_validation",
	};
#else
	const bool s_enableDebugLayers = false;
#endif

	const char* s_requiredInstanceExtensions[] = {
		// Required extensions for window surface creation
		"VK_KHR_surface",
		"VK_KHR_win32_surface",

		// TODO: Add other required extensions here

		// This must be the last extension
#ifdef APEX_VK_ENABLE_VALIDATION
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	const char* s_requiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

} // namespace detail


	VulkanContext* VulkanContext::s_pInstance = nullptr;

	void VulkanContext::InitializeVulkan(VulkanContext& obj, const char* app_name, Window* pwindow, bool enable_debugging)
	{
		s_pInstance = &obj;

		obj._initializeVulkan(app_name, pwindow, enable_debugging);
	}

	void VulkanContext::_initializeVulkan(const char* app_name, Window* pwindow, bool enable_debugging)
	{
		m_pWindow = pwindow;

		#ifdef APEX_VK_ENABLE_VALIDATION
		detail::s_enableDebugLayers = enable_debugging;
		#endif

		// Create a Vulkan instance
		_createInstance(app_name);

		// Add debug messenger for handling debug callbnacks
	}

	void VulkanContext::_cleanupVulkan()
	{
	}

	void VulkanContext::_createInstance(const char* app_name)
	{
		// TODO: Extract application and engine version info from env macros
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName   = app_name;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		applicationInfo.pEngineName        = "Apex Game Engine";
		applicationInfo.engineVersion      = VK_MAKE_VERSION(0, 2, 0);
		applicationInfo.apiVersion         = VK_API_VERSION_1_3;

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;

		// Check if validation layers are available
		axAssertMsg(!(detail::s_enableDebugLayers && !detail::check_validation_layer_support()),
			"Validation layers are requested but not supported!"
		);

		if (detail::s_enableDebugLayers)
		{
			instanceCreateInfo.enabledLayerCount   = static_cast<uint32>(std::size(detail::s_validationLayerNames));
			instanceCreateInfo.ppEnabledLayerNames = detail::s_validationLayerNames;

			VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = detail::debug_messenger_create_info();
			instanceCreateInfo.pNext = &debugMessengerCreateInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
			instanceCreateInfo.ppEnabledLayerNames = nullptr;
		}

		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32>(std::size(detail::s_requiredInstanceExtensions));
		instanceCreateInfo.ppEnabledExtensionNames = detail::s_requiredInstanceExtensions;

		axVerifyMsg(VK_SUCCESS == vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance),
			"Failed to create Vulkan instance!"
		);
	}


	// namespace detail :: definitions
	VkDebugUtilsMessengerCreateInfoEXT detail::debug_messenger_create_info()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = 
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = detail::vulkan_debug_messenger_callback;
		createInfo.pUserData = nullptr;

		return createInfo;
	}

	bool detail::check_validation_layer_support()
	{
		// TODO: implement checking for support of validation layers
		return true;
	}

	VkBool32 detail::vulkan_debug_messenger_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
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
} // namespace gfx
} // namespace apex
