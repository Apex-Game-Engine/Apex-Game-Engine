#pragma once
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"

namespace apex {
namespace vk {
	struct VulkanQueueFamilyIndices;
	struct VulkanSwapchainSupportDetails;

	struct VulkanSwapchain
	{
		VkSwapchainKHR         swapchain{};

		VkSurfaceFormatKHR     surfaceFormat{};
		VkPresentModeKHR       presentMode{};
		VkExtent2D             extent{};

		AxArray<VkImage>       images;
		AxArray<VkImageView>   imageViews;
		AxArray<VkFramebuffer> framebuffers;

		void create(
			VkDevice device,
			VkSurfaceKHR surface,
			VulkanSwapchainSupportDetails const& swapchain_support_details,
			uint32 width,
			uint32 height,
			VulkanQueueFamilyIndices const& queue_indices,
			VkAllocationCallbacks const* pAllocator);
		void createImageViews(VkDevice device, VkAllocationCallbacks const* pAllocator);
		void createFramebuffers(VkDevice device, VkRenderPass render_pass, VkImageView* depth_image_view, VkAllocationCallbacks const* pAllocator);
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);

	protected:
		static auto chooseSurfaceFormat(AxArray<VkSurfaceFormatKHR> const& available_formats) -> VkSurfaceFormatKHR;
		static auto choosePresentMode(AxArray<VkPresentModeKHR> const& available_present_modes) -> VkPresentModeKHR;
		static auto chooseExtent(VkSurfaceCapabilitiesKHR const& capabilities, uint32 width, uint32 height) -> VkExtent2D;
	};

}
}
