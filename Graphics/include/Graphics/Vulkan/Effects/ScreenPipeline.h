﻿#pragma once

#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

namespace apex {
namespace vk {

	struct ScreenPipeline : public VulkanGraphicsPipeline
	{
		void create(
			VkDevice device,
			gfx::VertexDescription const& vertex_descriptor,
			VulkanShaderStagesDesc const& shader_stages_desc,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) override;
	};

}
}