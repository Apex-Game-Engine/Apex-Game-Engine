#include "Graphics/Vulkan/VulkanDescriptorSetLayout.h"

#include "Core/Asserts.h"

namespace apex::vk {

	void VulkanDescriptorSetLayout::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyDescriptorSetLayout(device, layout, pAllocator);
	}
}
