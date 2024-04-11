#include "Graphics/Vulkan/Effects/CameraDescriptorSetLayout.h"

namespace apex::vk {

	void CameraDescriptorSetLayoutRecipe::build(VulkanDescriptorSetLayoutBuilder& layout)
	{
		layout.setBindingCount(layout.getBindingCount() + 2);
		layout
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
	}
}
