#include "Graphics/Vulkan/Effects/CameraDescriptorSetLayout.h"

#include "Core/Asserts.h"

namespace apex::vk {

	void CameraDescriptorSetLayout::create(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		VkDescriptorSetLayoutBinding layoutBindings[] = {
			// UBO binding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = nullptr
			},
			// Sampler binding
			/*{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr
			}*/
		};

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32>(std::size(layoutBindings)),
			.pBindings = layoutBindings
		};

		axVerifyMsg(VK_SUCCESS == vkCreateDescriptorSetLayout(device, &layoutCreateInfo, pAllocator, &layout),
			"Failed to create descriptor set layout!"
		);
	}
}
