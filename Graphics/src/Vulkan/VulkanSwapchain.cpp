#include "Graphics/Vulkan/VulkanSwapchain.h"
#include "Graphics/Vulkan/VulkanCommon.h"

namespace apex::vk {

	void VulkanSwapchain::create(
		VkDevice device,
		VkSurfaceKHR surface,
		VulkanSwapchainSupportDetails const& swapchain_support_details,
		uint32 width, uint32 height,
		VulkanQueueFamilyIndices const& queue_indices,
		VkAllocationCallbacks const* pAllocator)
	{
		surfaceFormat = chooseSurfaceFormat(swapchain_support_details.formats);
		presentMode = choosePresentMode(swapchain_support_details.presentModes);
		extent = chooseExtent(swapchain_support_details.capabilities, width, height);

		uint32 imageCount = swapchain_support_details.capabilities.minImageCount + 1;
		if (swapchain_support_details.capabilities.maxImageCount > 0 && imageCount > swapchain_support_details.capabilities.maxImageCount)
		{
			imageCount--;
		}
		axAssert(imageCount > 0);

		bool separateQueues = queue_indices.graphicsFamily.value() != queue_indices.presentFamily.value();
		uint32 queueFamilyIndices[] = { queue_indices.graphicsFamily.value(), queue_indices.presentFamily.value() };

		VkSwapchainCreateInfoKHR createInfo {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,

			// Image options
			.minImageCount = imageCount,
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,

			// TODO: Consider changing to VK_IMAGE_USAGE_TRANSFER_DST_BIT for blitting from postprocessing framebuffer image
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

			// If the graphics queue and present queue are separate then image needs to enable concurrent access
			.imageSharingMode = separateQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = separateQueues ? 2u : 0,
			.pQueueFamilyIndices = separateQueues ? queueFamilyIndices : nullptr,

			// currentTransform : no transformation
			.preTransform = swapchain_support_details.capabilities.currentTransform,

			// QPAQUE : ignore alpha
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

			.presentMode = presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		axVerifyMsg(VK_SUCCESS == vkCreateSwapchainKHR(device, &createInfo, pAllocator, &swapchain),
			"Failed to craete swapchain!"
		);

		// Store the swapchain images for reference during rendering
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
	}

	void VulkanSwapchain::createImageViews(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		imageViews.resize(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			VkImageViewCreateInfo createInfo {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfaceFormat.format,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			axVerifyMsg(VK_SUCCESS == vkCreateImageView(device, &createInfo, pAllocator, &imageViews[i]),
				"Failed to create swapchain image view!"
			);
		}
	}

	void VulkanSwapchain::createFramebuffers(VkDevice device, VkRenderPass render_pass, VkImageView * depth_image_view, VkAllocationCallbacks const* pAllocator)
	{
		framebuffers.resize(imageViews.size());

		for (size_t i = 0; i < imageViews.size(); i++)
		{
			AxArray<VkImageView> attachments;
			attachments.reserve(2);
			attachments.append(imageViews[i]);
			if (depth_image_view)
				attachments.append(*depth_image_view);

			VkFramebufferCreateInfo createInfo {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = render_pass,
				.attachmentCount = static_cast<uint32>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = extent.width,
				.height = extent.height,
				.layers = 1
			};

			axVerifyMsg(VK_SUCCESS == vkCreateFramebuffer(device, &createInfo, pAllocator, &framebuffers[i]),
				"Failed to create framebuffer!"
			);
		}
	}

	void VulkanSwapchain::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		for (auto framebuffer : framebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, pAllocator);
		}

		for (auto imageView : imageViews)
		{
			vkDestroyImageView(device, imageView, pAllocator);
		}

		vkDestroySwapchainKHR(device, swapchain, pAllocator);
	}

	auto VulkanSwapchain::chooseSurfaceFormat(AxArray<VkSurfaceFormatKHR> const& available_formats) -> VkSurfaceFormatKHR
	{
		constexpr static VkSurfaceFormatKHR kPrefferedFormat { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		// Check if preferred format is available
		for (VkSurfaceFormatKHR const& format : available_formats)
		{
			if (kPrefferedFormat.format == format.format && kPrefferedFormat.colorSpace == format.colorSpace)
			{
				return format;
			}
		}
		axWarn("Could not find preferred surface format B8G8R8A8_SRGB.");

		// TODO: Rank formats based on suitability and select highest ranked one
		// Otherwise select the first available format
		return available_formats[0];
	}

	auto VulkanSwapchain::choosePresentMode(AxArray<VkPresentModeKHR> const& available_present_modes) -> VkPresentModeKHR
	{
		// Check if the Mailbox (triple buffering) format is present
		for (VkPresentModeKHR const& presentMode : available_present_modes)
		{
			if (VK_PRESENT_MODE_MAILBOX_KHR == presentMode)
			{
				return presentMode;
			}
		}
		axWarn("Could not find present mode MAILBOX. Falling back to FIFO.");

		// Otherwise select the basic FIFO format (guaranteed to be available by the specification)
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	auto VulkanSwapchain::chooseExtent(VkSurfaceCapabilitiesKHR const& capabilities, uint32 width, uint32 height) -> VkExtent2D
	{
		// Check if the extent was set by Vulkan
		if (apex::constants::uint32_MAX != capabilities.currentExtent.width)
		{
			return capabilities.currentExtent;
		}

		// Otherwise find the best set of extents to match the window width and height in pixels
		VkExtent2D actualExtent { width, height };

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
