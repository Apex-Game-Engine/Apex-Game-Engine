#pragma once

#include <vma.h>
#include <vulkan/vulkan_core.h>

#include "VulkanCommon.h"
#include "Containers/AxArray.h"

namespace apex {
namespace vk {

	struct VulkanPhysicalDeviceFeatures
	{
		VulkanPhysicalDeviceFeatures()
		{
			setupChain();
			features13.pNext = nullptr;
		}

		VulkanPhysicalDeviceFeatures(VulkanPhysicalDeviceFeatures const& other)
			: features{other.features}
			, features11{other.features11}
			, features12{other.features12}
			, features13{other.features13}
		{
			setupChain();
		}

		VulkanPhysicalDeviceFeatures& operator=(VulkanPhysicalDeviceFeatures const& other)
		{
			features = other.features;
			features11 = other.features11;
			features12 = other.features12;
			features13 = other.features13;
			setupChain();
			return *this;
		}

		void attach(void* pNext)
		{
			auto ppNext = reinterpret_cast<VkBaseOutStructure*>(&features13);
			while (ppNext->pNext != nullptr)
			{
				ppNext = ppNext->pNext;
			}
			ppNext->pNext = static_cast<VkBaseOutStructure*>(pNext);
		}

		void setupChain()
		{
			features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features.pNext = &features11;
			features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			features11.pNext = &features12;
			features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			features12.pNext = &features13;
			features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		}

		VkPhysicalDeviceFeatures2        features{};
		VkPhysicalDeviceVulkan11Features features11{};
		VkPhysicalDeviceVulkan12Features features12{};
		VkPhysicalDeviceVulkan13Features features13{};
	};

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

		VkCommandPool    transferCommandPool{};
		
		VmaAllocator		 vmaAllocator{};

		VkPhysicalDeviceProperties       physicalDeviceProperties{};
		VulkanPhysicalDeviceFeatures     physicalDeviceFeatures{};
		VulkanPhysicalDeviceFeatures     requiredDeviceFeatures{};
		AxArray<VkExtensionProperties>   availableExtensions{};
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};


		void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VulkanPhysicalDeviceFeatures const& required_features);
		void createLogicalDevice(VkAllocationCallbacks const* pAllocator);
		void createCommandPools(VkAllocationCallbacks const* pAllocator);
		void destroy(VkAllocationCallbacks const* pAllocator);

		auto beginOneShotCommandBuffer(VkCommandPool command_pool) const -> VkCommandBuffer;
		auto findSuitableMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties) const -> uint32;
		void createMemoryAllocator(VkInstance instance, VkAllocationCallbacks const* pAllocator);

	protected:
		bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VulkanPhysicalDeviceFeatures const& required_features);
	};

}
}
