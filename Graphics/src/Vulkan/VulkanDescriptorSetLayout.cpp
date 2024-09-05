#include "Graphics/Vulkan/VulkanDescriptorSetLayout.h"

#include "Core/Asserts.h"

namespace apex::vk {

	void VulkanDescriptorSetLayout::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyDescriptorSetLayout(device, layout, pAllocator);
	}

	VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilder()
		: m_createInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = 0,
			.pBindings = nullptr
		}
	{
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::setBindingCount(uint32_t bindingCount)
	{
		m_bindings.reserve(bindingCount);
		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		uint32_t descriptorCount,
		VkShaderStageFlags stageFlags,
		const VkSampler* pImmutableSamplers)
	{
		VkDescriptorSetLayoutBinding bindingInfo = {
			.binding = binding,
			.descriptorType = descriptorType,
			.descriptorCount = descriptorCount,
			.stageFlags = stageFlags,
			.pImmutableSamplers = pImmutableSamplers,
		};

		m_bindings.emplace_back(bindingInfo);
		return *this;
	}				

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addUniformBuffer(
		uint32_t binding,
		uint32_t descriptorCount,
		VkShaderStageFlags stageFlags)
	{
		return addBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount, stageFlags);
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addStorageBuffer(
		uint32_t binding,
		uint32_t descriptorCount,
		VkShaderStageFlags stageFlags)
	{
		return addBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorCount, stageFlags);
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addCombinedImageSampler(
		uint32_t binding,
		uint32_t descriptorCount,
		VkShaderStageFlags stageFlags,
		const VkSampler * pImmutableSamplers)
	{
		return addBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount, stageFlags, pImmutableSamplers);
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addStorageImage(uint32_t binding,
		uint32_t descriptorCount, VkShaderStageFlags stageFlags)
	{
		return addBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorCount, stageFlags);
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::setFlags(VkDescriptorSetLayoutCreateFlags flags)
	{
		m_createInfo.flags = flags;
		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::setNext(const void* pNext)
	{
		m_createInfo.pNext = pNext;
		return *this;
	}

	VulkanDescriptorSetLayout VulkanDescriptorSetLayoutBuilder::build(
		VkDevice device,
		VkAllocationCallbacks const* pAllocator)
	{
		m_createInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
		m_createInfo.pBindings = m_bindings.data();

		VkDescriptorSetLayout layout;
		axVerifyMsg(VK_SUCCESS == vkCreateDescriptorSetLayout(device, &m_createInfo, pAllocator, &layout),
			"Failed to create descriptor set layout"
		);

		return VulkanDescriptorSetLayout { .layout = layout };
	}
}
