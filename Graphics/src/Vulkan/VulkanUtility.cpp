#include "Graphics/Vulkan/VulkanUtility.h"
#include "Graphics/Vulkan/VulkanCommon.h"

#include "Core/Types.h"
#include "Containers/AxArray.h"

#include <vulkan/vulkan_core.h>


namespace apex::vk {
	
	bool check_validation_layer_support(const char* validationLayerNames[], size_t validationLayerCount)
	{
		uint32 instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

		AxArray<VkLayerProperties> instanceLayerProperties;
		instanceLayerProperties.resize(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());

		AxArray<bool> validationLayersPresent;
		validationLayersPresent.resize(instanceLayerCount, false);

		size_t numValidationLayersPresent = 0;

		for (VkLayerProperties& layer : instanceLayerProperties)
		{
			for (size_t i = 0; i < validationLayerCount; i++)
			{
				if (!validationLayersPresent[i] && strcmp(layer.layerName, validationLayerNames[i]) == 0)
				{
					validationLayersPresent[i] = true;
					numValidationLayersPresent++;
				}
			}
		}

		return numValidationLayersPresent == validationLayerCount;
	}

	auto query_swapchain_support_details(VkPhysicalDevice device, VkSurfaceKHR surface) -> VulkanSwapchainSupportDetails
	{
		VulkanSwapchainSupportDetails supportDetails;

		// Query the physical device's surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &supportDetails.capabilities);

		// Query the physical device's supported surface formats
		uint32 formatsCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);

		if (0 != formatsCount)
		{
			supportDetails.formats.resize(formatsCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, supportDetails.formats.data());
		}

		// Query the physical device's supported presentation modes
		uint32 presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);

		if (0 != presentModesCount)
		{
			supportDetails.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, supportDetails.presentModes.data());
		}

		return supportDetails;
	}

}
