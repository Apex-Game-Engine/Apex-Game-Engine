#pragma once
#include "Core/Types.h"

#include <optional>
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"

namespace apex {
namespace vk {

	struct VulkanQueueFamilyIndices
	{
		std::optional<uint32> graphicsFamily; // For rendering
		std::optional<uint32> presentFamily;  // For displaying
		std::optional<uint32> transferFamily; // For moving/copying
		std::optional<uint32> computeFamily;  // For compute shaders

		[[nodiscard]] bool isRenderComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}

		[[nodiscard]] bool isComplete() const
		{
			return isRenderComplete() && computeFamily.has_value();
		}
	};

	struct VulkanSwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		AxArray<VkSurfaceFormatKHR> formats;
		AxArray<VkPresentModeKHR> presentModes;
	};

}
}
