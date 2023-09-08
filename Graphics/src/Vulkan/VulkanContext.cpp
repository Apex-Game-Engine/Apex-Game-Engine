#include "Graphics/Vulkan/VulkanContext.h"

#include "Core/Asserts.h"
#include "Core/Logging.h"
#include "Graphics/Vulkan/VulkanFunctions.h"

#include <vulkan/vulkan.h>

#include "Graphics/Vulkan/VulkanConfig.h"
#include "Graphics/Vulkan/VulkanDebugMessenger.h"
#include "Graphics/Vulkan/VulkanUtility.h"
#include "Graphics/Window/Window.h"

namespace apex::vk {

	void VulkanContext::initialize(const char* app_name, Window* p_window, bool enable_debugging)
	{
		axInfo("Initializing Vulkan");

		_initializeVulkan(app_name, p_window, enable_debugging);
		m_isInitialized = true;
	}

	void VulkanContext::shutdown()
	{
		_cleanupVulkan();
		m_isInitialized = false;
	}

	void VulkanContext::onWindowResize(uint32 width, uint32 height)
	{
		_recreateSwapchain(width, height);
	}

	void VulkanContext::handleWindowResize()
	{
		// Get window width and height
		int width, height;
		m_pWindow->getFramebufferSize(width, height);

		_recreateSwapchain(width, height);
	}

	void VulkanContext::_initializeVulkan(const char* app_name, Window* p_window, bool enable_debugging)
	{
		m_pWindow = p_window;

#ifdef APEX_VK_ENABLE_VALIDATION
		VulkanConfig::kEnableDebugLayers = enable_debugging;
#endif

		// Create a Vulkan instance
		_createInstance(app_name);

		// Add debug messenger for handling debug callbacks
		m_debugMessenger.create(m_instance, nullptr);

		// Create a window surface for presenting on screen
		_createSurface(p_window);

		// Select a physical device
		m_device.selectPhysicalDevice(m_instance, m_surface, { .geometryShader = true });

		// Create a logical device
		m_device.createLogicalDevice(nullptr);

		// Get window width and height
		int width, height;
		m_pWindow->getFramebufferSize(width, height);

		// Create swapchain
		auto swapchainSupportDetails = query_swapchain_support_details(m_device.physicalDevice, m_surface);
		m_swapchain.create(m_device.logicalDevice, m_surface, swapchainSupportDetails, width, height, m_device.queueFamilyIndices, nullptr);
		m_swapchain.createImageViews(m_device.logicalDevice, nullptr);

		// Create command pool
		m_device.createCommandPools(nullptr);
	}

	void VulkanContext::_cleanupVulkan()
	{
		m_swapchain.destroy(m_device.logicalDevice, nullptr);

		m_device.destroy(nullptr);

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

		if (VulkanConfig::kEnableDebugLayers)
		{
			m_debugMessenger.destroy(m_instance, nullptr);
		}

		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanContext::_createInstance(const char* app_name)
	{
		// TODO: Extract application and engine version info from env macros
		VkApplicationInfo applicationInfo {
			.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName   = app_name,
			.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
			.pEngineName        = "Apex Game Engine",
			.engineVersion      = VK_MAKE_VERSION(0, 2, 0),
			.apiVersion         = VK_API_VERSION_1_3,
		};

		VkInstanceCreateInfo instanceCreateInfo {
			.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &applicationInfo,
		};

		// Check if validation layers are available
		axAssertMsg(!(VulkanConfig::kEnableDebugLayers && !vk::check_validation_layer_support(VulkanConfig::kValidationLayerNames, std::size(VulkanConfig::kValidationLayerNames))),
		            "Validation layers are requested but not supported!"
		);

		const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = m_debugMessenger.getCreateInfo();

		if (VulkanConfig::kEnableDebugLayers)
		{
			instanceCreateInfo.enabledLayerCount   = static_cast<uint32>(std::size(VulkanConfig::kValidationLayerNames));
			instanceCreateInfo.ppEnabledLayerNames = VulkanConfig::kValidationLayerNames;
			instanceCreateInfo.pNext = &debugMessengerCreateInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
			instanceCreateInfo.ppEnabledLayerNames = nullptr;
		}

		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32>(std::size(VulkanConfig::kRequiredInstanceExtensions));
		instanceCreateInfo.ppEnabledExtensionNames = VulkanConfig::kRequiredInstanceExtensions;

		axVerifyMsg(VK_SUCCESS == vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance),
		            "Failed to create Vulkan instance!"
		);
	}

	void VulkanContext::_createSurface(Window* p_window)
	{
	#ifdef APEX_PLATFORM_WIN32
		const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(NULL),
			.hwnd = static_cast<HWND>(m_pWindow->getOsWindowHandle())
		};

		axVerifyMsg(VK_SUCCESS == vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface),
		            "Failed to create window surface!"
		);
	#else
		#error Not implemented for this platform!
	#endif
	}

	void VulkanContext::_recreateSwapchain(uint32 width, uint32 height)
	{
		vkDeviceWaitIdle(m_device.logicalDevice);

		// Destroy old swapchain
		m_swapchain.destroy(m_device.logicalDevice, nullptr);

		// Create swapchain
		auto swapchainSupportDetails = query_swapchain_support_details(m_device.physicalDevice, m_surface);
		m_swapchain.create(m_device.logicalDevice, m_surface, swapchainSupportDetails, width, height, m_device.queueFamilyIndices, nullptr);
		m_swapchain.createImageViews(m_device.logicalDevice, nullptr);
	}
}
