#include "Graphics/Vulkan/VulkanPipeline.h"

#include "Graphics/Vulkan/VulkanShader.h"

namespace apex::vk {
	
	void VulkanPipeline::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyPipeline(device, pipeline, pAllocator);
		vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
	}

}
