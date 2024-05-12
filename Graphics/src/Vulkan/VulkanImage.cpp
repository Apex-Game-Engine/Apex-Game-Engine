#include "Graphics/Vulkan/VulkanImage.h"

namespace apex {
namespace vk {

	namespace {
		constexpr VkImageViewType getImageViewType(VkImageType image_type)
		{
			switch (image_type) {
			case VK_IMAGE_TYPE_1D:
				return VK_IMAGE_VIEW_TYPE_1D;
			case VK_IMAGE_TYPE_2D:
				return VK_IMAGE_VIEW_TYPE_2D;
			case VK_IMAGE_TYPE_3D:
				return VK_IMAGE_VIEW_TYPE_3D;
			default:
				axAssertMsg(false, "Invalid image type");
				return VK_IMAGE_VIEW_TYPE_2D;
			}
		}
	}


	void VulkanImage::destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyImageView(device.logicalDevice, imageView, pAllocator);
		vmaDestroyImage(device.vmaAllocator, image, allocation);
	}

	VulkanImageBuilder::VulkanImageBuilder()
		: m_imageCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
		}
		, m_allocationCreateInfo {
			.flags = 0,
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
		}
		, m_imageViewCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
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
				.layerCount = 1,
			},
		}
	{
	}

	VulkanImageBuilder& VulkanImageBuilder::setImageType(VkImageType image_type)
	{
		m_imageCreateInfo.imageType = image_type;
		m_imageViewCreateInfo.viewType = getImageViewType(image_type);
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setFormat(VkFormat format)
	{
		m_imageCreateInfo.format = format;
		m_imageViewCreateInfo.format = format;
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
			m_imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setExtent(VkExtent3D extent)
	{
		m_imageCreateInfo.extent = extent;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setExtent(uint32 width, uint32 height, uint32 depth)
	{
		return setExtent({ width, height, depth });
	}

	VulkanImageBuilder& VulkanImageBuilder::setMipLevels(uint32 mip_levels)
	{
		m_imageCreateInfo.mipLevels = mip_levels;
		m_imageViewCreateInfo.subresourceRange.levelCount = mip_levels;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setArrayLayers(uint32 array_layers)
	{
		m_imageCreateInfo.arrayLayers = array_layers;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setSamples(VkSampleCountFlagBits samples)
	{
		m_imageCreateInfo.samples = samples;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setTiling(VkImageTiling tiling)
	{
		m_imageCreateInfo.tiling = tiling;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setUsage(VkImageUsageFlags usage)
	{
		m_imageCreateInfo.usage = usage;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setSharingMode(VkSharingMode sharing_mode)
	{
		m_imageCreateInfo.sharingMode = sharing_mode;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setInitialLayout(VkImageLayout initial_layout)
	{
		m_imageCreateInfo.initialLayout = initial_layout;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setAllocationFlags(VmaAllocationCreateFlags flags)
	{
		m_allocationCreateInfo.flags = flags;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setMemoryUsage(VmaMemoryUsage memory_usage)
	{
		m_allocationCreateInfo.usage = memory_usage;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setAllocationRequiredFlags(VkMemoryPropertyFlags required_flags)
	{
		m_allocationCreateInfo.requiredFlags = required_flags;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setAspectFlags(VkImageAspectFlags aspect_flags)
	{
		m_imageViewCreateInfo.subresourceRange.aspectMask = aspect_flags;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::overrideImageViewCreateInfo(VkImageViewCreateInfo const& create_info)
	{
		m_imageViewCreateInfo = create_info;
		return *this;
	}

	VulkanImage VulkanImageBuilder::build(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		VulkanImage image;

		axAssertMsg(m_imageCreateInfo.mipLevels > 0, "Mip levels must be greater than 0");
		axAssertMsg(m_imageCreateInfo.arrayLayers > 0, "Array layers must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.width > 0, "Extent width must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.height > 0, "Extent height must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.depth > 0, "Extent depth must be greater than 0");

		axVerifyMsg(VK_SUCCESS == vmaCreateImage(device.vmaAllocator, &m_imageCreateInfo, &m_allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo),
			"Failed to create image"
		);

		image.extent = m_imageCreateInfo.extent;
		image.format = m_imageCreateInfo.format;

		m_imageViewCreateInfo.image = image.image;

		axVerifyMsg(VK_SUCCESS == vkCreateImageView(device.logicalDevice, &m_imageViewCreateInfo, pAllocator, &image.imageView),
			"Failed to create image view"
		);

		return image;
	}
}
}
