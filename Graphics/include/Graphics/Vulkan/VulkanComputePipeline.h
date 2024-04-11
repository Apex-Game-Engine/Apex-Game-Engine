#pragma once

#include <vulkan/vulkan_core.h>

#include "VulkanDevice.h"
#include "VulkanShader.h"

namespace apex {
namespace vk {

	struct VulkanComputePipeline
	{
		VkPipeline pipeline{};
		VkPipelineLayout pipelineLayout{};

		void create(VulkanDevice const& device, VkPipelineLayout const& layout, VulkanComputeShader const& compute_shader, VkAllocationCallbacks const* pAllocator);
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
	};

}
}
