﻿#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanShader.h"
#include "Graphics/Geometry/Vertex.h"

namespace apex {
namespace vk {
	struct VulkanShaderStagesDesc;

	struct VulkanGraphicsPipeline
	{
		VkPipeline pipeline{};
		VkPipelineLayout pipelineLayout{};

		virtual void create(
			VkDevice device,
			gfx::VertexDescription const& vertex_descriptor,
			VulkanShaderStagesDesc const& shader_stages_desc,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) = 0;

		virtual void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);

	protected:
		virtual void createPipelineLayout(
			VkDevice device,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
			VkAllocationCallbacks const* pAllocator);
	};

}
}
