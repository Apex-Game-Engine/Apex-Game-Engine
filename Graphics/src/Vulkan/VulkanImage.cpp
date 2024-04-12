#include "Graphics/Vulkan/VulkanImage.h"

namespace apex {
namespace vk {

	void VulkanImage::create2DImage(VulkanDevice const& device, VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
	{
		VkImageCreateInfo image_info {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = extent,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = usage_flags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
	}

	void VulkanImage::destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		vmaDestroyImage(device.m_allocator, image, allocation);
	}

	VulkanImageBuilder::VulkanImageBuilder()
		: m_imageCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
		}
		, m_allocationCreateInfo {
			.flags = 0,
			.requiredFlags = 0,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
		}
	{
	}

	VulkanImageBuilder& VulkanImageBuilder::setImageType(VkImageType image_type)
	{
		m_imageCreateInfo.imageType = image_type;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setFormat(VkFormat format)
	{
		m_imageCreateInfo.format = format;
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

	VulkanImage VulkanImageBuilder::build(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		VulkanImage image;

		axAssertMsg(m_imageCreateInfo.mipLevels > 0, "Mip levels must be greater than 0");
		axAssertMsg(m_imageCreateInfo.arrayLayers > 0, "Array layers must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.width > 0, "Extent width must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.height > 0, "Extent height must be greater than 0");
		axAssertMsg(m_imageCreateInfo.extent.depth > 0, "Extent depth must be greater than 0");

		axVerifyMsg(VK_SUCCESS == vmaCreateImage(device.m_allocator, &m_imageCreateInfo, &m_allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo),
			"Failed to create image"
		);

		image.extent = m_imageCreateInfo.extent;
		image.format = m_imageCreateInfo.format;

		return image;
	}
}
}
