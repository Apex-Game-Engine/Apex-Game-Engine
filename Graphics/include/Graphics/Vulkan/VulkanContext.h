#pragma once

#include <vulkan/vulkan_core.h>

namespace apex {
	struct Window;

namespace gfx {

	struct VulkanDeviceContext
	{
		VkPhysicalDevice m_physicalDevice{};
		VkDevice         m_device{};

		VkQueue          m_graphicsQueue{};
		VkQueue          m_presentQueue{};
		VkQueue          m_transferQueue{};

		VkPhysicalDeviceProperties m_physicalDeviceProperties{};
		VkPhysicalDeviceFeatures   m_physicalDeviceFeatures{};

	
	};

	struct VulkanContext
	{
		void initialize(const char* app_name, Window* pwindow, bool enable_debugging);
		void shutdown();

		static VulkanContext* GetInstance() { return s_pInstance; }

		VkInstance               m_instance{};
		VkDebugUtilsMessengerEXT m_debugMessenger{};
		Window*                  m_pWindow{};

		static VulkanContext *s_pInstance;
		
		// internal methods
		void _initializeVulkan(const char* app_name, Window* pwindow, bool enable_debugging);
		void _cleanupVulkan();

		void _createInstance(const char* app_name);
		void _createDebugMessenger();
		void _createSurface();
	};

}
}
