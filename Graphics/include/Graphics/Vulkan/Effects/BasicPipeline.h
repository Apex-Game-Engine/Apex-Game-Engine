#pragma once

#include "Graphics/Vulkan/VulkanPipeline.h"

namespace apex {
namespace vk {

	struct BasicPipeline : public VulkanPipeline
	{
		void create(
			VkDevice device,
			VulkanShaderStagesDesc const& shader_stages_desc,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) override;
		
	protected:
		void createPipelineLayout(VkDevice device, VkAllocationCallbacks const* pAllocator) override;
		void createDescriptorSetLayouts(VkDevice device, VkAllocationCallbacks const* pAllocator) override;
	};

}
}