#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanShader.h"

namespace apex {
namespace vk {
	struct VulkanShaderStagesDesc;

	struct VulkanPipeline
	{
		VkPipeline pipeline{};
		VkPipelineLayout pipelineLayout{};

		virtual void create(
			VkDevice device,
			VulkanShaderStagesDesc const& shader_stages_desc,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) = 0;

		virtual void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);

	protected:
		virtual void createPipelineLayout(
			VkDevice device,
			AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
			VkAllocationCallbacks const* pAllocator) = 0;
	};

}
}
