#pragma once
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"
#include "Containers/AxStringRef.h"

namespace apex {
namespace vk {

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout layout{};

		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
	};

	class VulkanDescriptorSetLayoutBuilder
	{
	public:
		VulkanDescriptorSetLayoutBuilder();
		~VulkanDescriptorSetLayoutBuilder() = default;

		VulkanDescriptorSetLayoutBuilder& setBindingCount(uint32 bindingCount);
		VulkanDescriptorSetLayoutBuilder& addBinding(uint32 binding, VkDescriptorType descriptorType, uint32 descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr);
		VulkanDescriptorSetLayoutBuilder& addUniformBuffer(uint32 binding, uint32 descriptorCount, VkShaderStageFlags stageFlags);
		VulkanDescriptorSetLayoutBuilder& addStorageBuffer(uint32 binding, uint32 descriptorCount, VkShaderStageFlags stageFlags);
		VulkanDescriptorSetLayoutBuilder& addCombinedImageSampler(uint32 binding, uint32 descriptorCount, VkShaderStageFlags stageFlags, const VkSampler * pImmutableSamplers = nullptr);
		VulkanDescriptorSetLayoutBuilder& addStorageImage(uint32 binding, uint32 descriptorCount, VkShaderStageFlags stageFlags);
		VulkanDescriptorSetLayoutBuilder& setFlags(VkDescriptorSetLayoutCreateFlags flags);
		VulkanDescriptorSetLayoutBuilder& setNext(const void* pNext);

		VulkanDescriptorSetLayout build(VkDevice device, VkAllocationCallbacks const* pAllocator);

		// getters
		VkDescriptorSetLayoutCreateInfo const& getCreateInfo() const { return m_createInfo; }
		AxArray<VkDescriptorSetLayoutBinding> const& getBindings() const { return m_bindings; }
		uint32 getBindingCount() const { return m_bindings.size(); }

	private:
		VkDescriptorSetLayoutCreateInfo m_createInfo{};
		AxArray<VkDescriptorSetLayoutBinding> m_bindings{};
		// TODO: add metadata for bindings
	};

	class VulkanDescriptorSetLayoutRecipe
	{
	public:
		virtual ~VulkanDescriptorSetLayoutRecipe() = default;
		virtual void build(VulkanDescriptorSetLayoutBuilder& layout) = 0;
	};

}
}
