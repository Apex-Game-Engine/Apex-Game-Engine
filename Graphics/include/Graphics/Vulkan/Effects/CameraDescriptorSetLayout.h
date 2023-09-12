#pragma once
#include "Graphics/Vulkan/VulkanDescriptorSetLayout.h"

namespace apex {
namespace vk {

	struct CameraDescriptorSetLayout : VulkanDescriptorSetLayout
	{
		void create(VkDevice device, VkAllocationCallbacks const* pAllocator) override;
	};

}
}
