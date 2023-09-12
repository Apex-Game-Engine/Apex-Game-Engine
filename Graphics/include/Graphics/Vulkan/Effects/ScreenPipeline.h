#pragma once

#include "Graphics/Vulkan/VulkanPipeline.h"

namespace apex {
namespace vk {

	struct ScreenPipeline : public VulkanPipeline
	{
		void create(
			VkDevice device,
			VulkanShaderStagesDesc const& shader_stages_desc,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) override;

	protected:
		void createPipelineLayout(
			VkDevice device,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			VkAllocationCallbacks const* pAllocator) override;
	};

}
}