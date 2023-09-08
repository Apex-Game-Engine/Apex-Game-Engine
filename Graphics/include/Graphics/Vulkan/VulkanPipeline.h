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
		AxArray<VkDescriptorSetLayout> descriptorSetLayouts{};

		virtual void create(
			VkDevice device,
			VulkanShaderStagesDesc const& shader_stages_desc,
			VkExtent2D swapchain_extent,
			VkRenderPass render_pass,
			VkAllocationCallbacks const* pAllocator) = 0;

		virtual void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);

	protected:
		virtual void createPipelineLayout(VkDevice device, VkAllocationCallbacks const* pAllocator) = 0;

		virtual void createDescriptorSetLayouts(VkDevice device, VkAllocationCallbacks const* pAllocator) = 0;
	};

}
}
