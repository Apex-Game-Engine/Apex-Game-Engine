#pragma once
#include <vulkan/vulkan_core.h>
#include <vma.h>

#include "VulkanDevice.h"
#include "Containers/AxArray.h"

namespace apex {
namespace vk {

	struct VulkanImage
	{
		VkImage           image;
		VmaAllocation     allocation;
		VmaAllocationInfo allocationInfo;
		VkExtent3D extent;
		VkFormat format;

		void create2DImage(VulkanDevice const& device, VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
		void destroy(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
	};
	
	class VulkanImageBuilder
	{
	public:
		VulkanImageBuilder();
		~VulkanImageBuilder() = default;

		// setters
		VulkanImageBuilder& setImageType(VkImageType image_type);
		VulkanImageBuilder& setFormat(VkFormat format);
		VulkanImageBuilder& setExtent(VkExtent3D extent);
		VulkanImageBuilder& setExtent(uint32 width, uint32 height, uint32 depth);
		VulkanImageBuilder& setMipLevels(uint32 mip_levels);
		VulkanImageBuilder& setArrayLayers(uint32 array_layers);
		VulkanImageBuilder& setSamples(VkSampleCountFlagBits samples);
		VulkanImageBuilder& setTiling(VkImageTiling tiling);
		VulkanImageBuilder& setUsage(VkImageUsageFlags usage);
		VulkanImageBuilder& setSharingMode(VkSharingMode sharing_mode);
		VulkanImageBuilder& setInitialLayout(VkImageLayout initial_layout);
		VulkanImageBuilder& setAllocationFlags(VmaAllocationCreateFlags flags);
		VulkanImageBuilder& setMemoryUsage(VmaMemoryUsage memory_usage);
		VulkanImageBuilder& setAllocationRequiredFlags(VkMemoryPropertyFlags required_flags);

		VulkanImage build(VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);

		// getters
		VkImageCreateInfo const& getImageCreateInfo() const { return m_imageCreateInfo; }
		VmaAllocationCreateInfo const& getAllocationCreateInfo() const { return m_allocationCreateInfo; }

	private:
		VkImageCreateInfo m_imageCreateInfo;
		VmaAllocationCreateInfo m_allocationCreateInfo;
	};

}	
}
