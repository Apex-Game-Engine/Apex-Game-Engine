#pragma once

#include "Containers/AxArray.h"

#include <vulkan/vulkan_core.h>

#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"

namespace apex {
	namespace gfx
	{
		class ForwardRenderer;
	}

	struct Window;

namespace vk {

	struct VulkanInstance
	{
		VkInstance m_instance{};
		AxArray<VkExtensionProperties> m_supportedInstanceExtensions{};
	};

	class VulkanContext
	{
	public:
		void initialize(const char* app_name, Window* p_window, bool enable_debugging);
		void shutdown();

		void onWindowResize(uint32 width, uint32 height);
		bool isInitialized() { return m_isInitialized; }

	private:
		void handleWindowResize();

		// internal methods
		void _initializeVulkan(const char* app_name, Window* p_window, bool enable_debugging);
		void _cleanupVulkan();

		void _createInstance(const char* app_name);
		void _createSurface(Window* p_window);
		void _recreateSwapchain(uint32 width, uint32 height);

	private:
		VkInstance           m_instance{};
		Window*              m_pWindow{};

		VulkanDebugMessenger m_debugMessenger{};
		VkSurfaceKHR         m_surface{};
		VulkanDevice         m_device{};
		VulkanSwapchain      m_swapchain{};

		bool                 m_isInitialized = false;


		friend class ::apex::gfx::ForwardRenderer;
	};

}
}
