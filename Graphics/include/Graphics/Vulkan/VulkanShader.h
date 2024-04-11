#pragma once
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"

namespace apex {
namespace vk {

	struct VulkanShaderStagesDesc
	{
		std::optional<const char*> vertShaderFile;
		std::optional<const char*> fragShaderFile;
		std::optional<const char*> geomShaderFile;
		std::optional<const char*> tescShaderFile;
		std::optional<const char*> teseShaderFile;
	};

	struct VulkanShaderStages
	{
		VkShaderModule vertShader{};
		VkShaderModule fragShader{};
		VkShaderModule geomShader{};
		VkShaderModule tescShader{};
		VkShaderModule teseShader{};

		uint32 numStages{};

		void create(VkDevice device, VulkanShaderStagesDesc const& shader_stages_desc, VkAllocationCallbacks const* pAllocator);
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
		auto getShaderStagesCreateInfos(VkDevice device) -> AxArray<VkPipelineShaderStageCreateInfo>;
	};

	struct VulkanComputeShader
	{
		VkShaderModule computeShader{};

		void create(VkDevice device, const char* computeShaderFile, VkAllocationCallbacks const* pAllocator);
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
		auto getShaderStageCreateInfo() -> VkPipelineShaderStageCreateInfo;
	};

}
}
