#include "Graphics/Vulkan/VulkanComputePipeline.h"

#include "Graphics/Vulkan/VulkanDevice.h"
#include "Graphics/Vulkan/VulkanShader.h"

namespace apex::vk {

	void VulkanComputePipeline::create(VulkanDevice const& device, VkPipelineLayout const& layout, VulkanComputeShader const& compute_shader, VkAllocationCallbacks const* pAllocator)
	{
		VkComputePipelineCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		create_info.layout = layout;
		create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		create_info.stage.module = compute_shader.computeShader;
		create_info.stage.pName = "main";

		axVerifyMsg(VK_SUCCESS == vkCreateComputePipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &create_info, pAllocator, &pipeline),
			"Failed to create compute pipeline"
		);

		pipelineLayout = layout;
	}

	void VulkanComputePipeline::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyPipeline(device, pipeline, pAllocator);
		//vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
	}
}
