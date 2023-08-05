#include "Graphics/Vulkan/VulkanContext.h"

#include "Core/Asserts.h"
#include "Core/Logging.h"
#include "Graphics/Vulkan/VulkanFunctions.h"

#include <vulkan/vulkan.h>

#include "Graphics/Vulkan/VulkanDebug.h"
#include "Graphics/Vulkan/VulkanUtility.h"

namespace apex {
namespace gfx {

namespace detail {


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
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

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

	void VulkanContext::initialize(const char* app_name, Window* p_window, bool enable_debugging)
	{
		s_pInstance = this;

		_initializeVulkan(app_name, p_window, enable_debugging);
	}

	void VulkanContext::shutdown()
	{
		// TODO:
	}

	void VulkanContext::_initializeVulkan(const char* app_name, Window* p_window, bool enable_debugging)
	{
		m_pWindow = p_window;

	#ifdef APEX_VK_ENABLE_VALIDATION
		detail::kEnableDebugLayers = enable_debugging;
	#endif

		// Create a Vulkan instance
		_createInstance(app_name);

		// Add debug messenger for handling debug callbacks
		_createDebugMessenger();

		// Create a window surface for presenting on screen
		_createSurface(p_window);
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
		axAssertMsg(!(detail::kEnableDebugLayers && !vk::check_validation_layer_support(detail::kValidationLayerNames, std::size(detail::kValidationLayerNames))),
			"Validation layers are requested but not supported!"
		);

		const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = vk::debug_messenger_create_info();

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
		VkDebugUtilsMessengerCreateInfoEXT createInfo = vk::debug_messenger_create_info();

		axAssertMsg(VK_SUCCESS == vk::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger),
			"Failed to create Vulkan debug utils messenger!"
		);
	}

	void VulkanContext::_createSurface(Window* p_window)
	{

	}

} // namespace gfx
} // namespace apex
