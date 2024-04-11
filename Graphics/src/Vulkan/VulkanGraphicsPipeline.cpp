#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

#include "Graphics/Vulkan/VulkanShader.h"

namespace apex::vk {
	
	void VulkanGraphicsPipeline::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyPipeline(device, pipeline, pAllocator);
		vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
	}

	void VulkanGraphicsPipeline::createPipelineLayout(
		VkDevice device,
		AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
		AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
		VkAllocationCallbacks const* pAllocator)
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<uint32>(descriptor_set_layouts.count),
			.pSetLayouts = descriptor_set_layouts.data,
			.pushConstantRangeCount = static_cast<uint32>(push_constant_ranges.count),
			.pPushConstantRanges = push_constant_ranges.data
		};

		axVerifyMsg(VK_SUCCESS == vkCreatePipelineLayout(device, &layoutCreateInfo, pAllocator, &pipelineLayout),
			"Failed to create pipeline layout!"
		);
	}
}
