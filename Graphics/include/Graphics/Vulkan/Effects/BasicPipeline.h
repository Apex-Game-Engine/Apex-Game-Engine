#pragma once

#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

namespace apex {
namespace vk {

	struct BasicPipeline : public VulkanGraphicsPipeline
	{
		void create(
			VkDevice device,
			gfx::VertexDescription const& vertex_description,
			VulkanShaderStagesDesc const& shader_stages_desc,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) override;
	};

}
}