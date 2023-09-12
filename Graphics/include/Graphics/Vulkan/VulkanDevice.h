#pragma once

#include <vulkan/vulkan_core.h>

#include "VulkanCommon.h"
#include "Containers/AxArray.h"

namespace apex {
namespace vk {

	struct VulkanDevice
	{
		VkPhysicalDevice physicalDevice{};
		VkDevice         logicalDevice{};

		VulkanQueueFamilyIndices queueFamilyIndices{};
		bool             hasDedicatedTransferQueue{};

		VkQueue          graphicsQueue{};
		VkQueue          presentQueue{};
		VkQueue          transferQueue{};
		VkQueue          computeQueue{};

		VkCommandPool    commandPool{};
		VkCommandPool    transferCommandPool{};

		VkPhysicalDeviceProperties       physicalDeviceProperties{};
		VkPhysicalDeviceFeatures         physicalDeviceFeatures{};
		VkPhysicalDeviceFeatures         requiredDeviceFeatures{};
		AxArray<VkExtensionProperties>   availableExtensions{};
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};


		void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDeviceFeatures const& required_features);
		void createLogicalDevice(VkAllocationCallbacks const* pAllocator);
		void createCommandPools(VkAllocationCallbacks const* pAllocator);
		void destroy(VkAllocationCallbacks const* pAllocator);

		auto beginOneShotCommandBuffer(VkCommandPool command_pool) const -> VkCommandBuffer;
		auto findSuitableMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties) const -> uint32;

	protected:
		bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceFeatures required_features);
	};

}
}
