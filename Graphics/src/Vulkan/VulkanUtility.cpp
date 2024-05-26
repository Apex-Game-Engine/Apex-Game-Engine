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

	void transition_image_layout(
		VkCommandBuffer command_buffer,
		VkImage image,
		VkImageLayout old_layout,
		VkImageLayout new_layout)
	{
		auto aspectFlags = static_cast<VkImageAspectFlags>(new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
					                                              ? VK_IMAGE_ASPECT_DEPTH_BIT
					                                              : VK_IMAGE_ASPECT_COLOR_BIT);

		VkImageMemoryBarrier2 imageBarrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.image = image,
			.subresourceRange = {
				.aspectMask = aspectFlags,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS
			},
		};

		VkDependencyInfo dependencyInfo {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageBarrier,
		};

		vkCmdPipelineBarrier2(command_buffer, &dependencyInfo);
	}
}
