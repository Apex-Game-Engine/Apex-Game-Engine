#pragma once
#include "Graphics/Vulkan/VulkanDescriptorSetLayout.h"

namespace apex {
namespace vk {

	class CameraDescriptorSetLayoutRecipe : public VulkanDescriptorSetLayoutRecipe
	{
	public:
		void build(VulkanDescriptorSetLayoutBuilder& layout) override;
	};

}
}
