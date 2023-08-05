#pragma once

#include "Containers/AxArray.h"

#include <vulkan/vulkan_core.h>

namespace apex {
	struct Window;

namespace gfx {

	struct VulkanInstanceContext
	{
		VkInstance m_instance{};
		AxArray<VkExtensionProperties> m_supportedInstanceExtensions{};
	};

	class VulkanContext
	{
	public:
		void initialize(const char* app_name, Window* p_window, bool enable_debugging);
		void shutdown();

		static VulkanContext* GetInstance() { return s_pInstance; }

	private:
		// internal methods
		void _initializeVulkan(const char* app_name, Window* p_window, bool enable_debugging);
		void _cleanupVulkan();

		void _createInstance(const char* app_name);
		void _createDebugMessenger();
		void _createSurface(Window* p_window);

	private:
		VkInstance               m_instance{};
		
		VkDebugUtilsMessengerEXT m_debugMessenger{};
		Window*                  m_pWindow{};

		static VulkanContext *s_pInstance;

	};

}
}
