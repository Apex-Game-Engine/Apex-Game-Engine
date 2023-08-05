#pragma once

#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"

namespace apex {
namespace gfx {

	struct VulkanDevice
	{
		VkPhysicalDevice m_physicalDevice{};
		VkDevice         m_logicalDevice{};

		VkQueue          m_graphicsQueue{};
		VkQueue          m_presentQueue{};
		VkQueue          m_transferQueue{};

		VkPhysicalDeviceProperties       m_deviceProperties{};
		VkPhysicalDeviceFeatures         m_supportedDeviceFeatures{};
		VkPhysicalDeviceFeatures         m_enabledDeviceFeatures{};
		VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties{};
		AxArray<VkExtensionProperties>   m_availableExtensions{};
	};

}
}
