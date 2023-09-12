#pragma once
#include <vulkan/vulkan_core.h>

namespace apex {
namespace vk {

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout layout{};

		virtual void create(VkDevice device, VkAllocationCallbacks const* pAllocator) = 0;
		virtual void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
	};

}
}
