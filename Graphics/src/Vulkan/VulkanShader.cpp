#include "Graphics/Vulkan/VulkanShader.h"

#include "Core/Files.h"

namespace apex::vk {

	namespace detail
	{
		static auto createShaderModule(VkDevice device, AxArray<char> const& code, VkAllocationCallbacks const* pAllocator) -> VkShaderModule;
	}

	void VulkanShaderStages::create(
		VkDevice device,
		VulkanShaderStagesDesc const& shader_stages_desc,
		VkAllocationCallbacks const* pAllocator)
	{
		axAssertMsg(shader_stages_desc.vertShaderFile.has_value(), "Cannot create pipeline without vertex shader!");
		axAssertMsg(shader_stages_desc.fragShaderFile.has_value(), "Cannot create pipeline without fragment shader!");

		constexpr auto FILE_MODE = FileModeFlags::eRead | FileModeFlags::eOpenExisting | FileModeFlags::eBinary;

		const auto vertShaderCode = readFile(shader_stages_desc.vertShaderFile.value(), FILE_MODE);
		const auto fragShaderCode = readFile(shader_stages_desc.fragShaderFile.value(), FILE_MODE);

		vertShader = detail::createShaderModule(device, vertShaderCode, pAllocator);
		fragShader = detail::createShaderModule(device, fragShaderCode, pAllocator);

		numStages += 2;

		if (shader_stages_desc.geomShaderFile.has_value())
		{
			const auto geomShaderCode = readFile(shader_stages_desc.geomShaderFile.value(), FILE_MODE);
			geomShader = detail::createShaderModule(device, geomShaderCode, pAllocator);
			numStages++;
		}

		if (shader_stages_desc.tescShaderFile.has_value())
		{
			const auto tescShaderCode = readFile(shader_stages_desc.tescShaderFile.value(), FILE_MODE);
			tescShader = detail::createShaderModule(device, tescShaderCode, pAllocator);
			numStages++;
		}

		if (shader_stages_desc.tescShaderFile.has_value())
		{
			const auto tescShaderCode = readFile(shader_stages_desc.tescShaderFile.value(), FILE_MODE);
			tescShader = detail::createShaderModule(device, tescShaderCode, pAllocator);
			numStages++;
		}
	}

	void VulkanShaderStages::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyShaderModule(device, vertShader, pAllocator);
		vkDestroyShaderModule(device, fragShader, pAllocator);
		if (geomShader)
			vkDestroyShaderModule(device, geomShader, pAllocator);
		if (tescShader)
			vkDestroyShaderModule(device, tescShader, pAllocator);
		if (teseShader)
			vkDestroyShaderModule(device, teseShader, pAllocator);
	}

	auto VulkanShaderStages::getShaderStagesCreateInfos(VkDevice device) -> AxArray<VkPipelineShaderStageCreateInfo>
	{
		AxArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
		shaderStageCreateInfos.resize(numStages);

		uint32 currentStage = 0;

		// Create vertex shader stage
		shaderStageCreateInfos[currentStage++] = {
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShader,
			.pName  = "main"
		};

		// Create geometry shader stage
		if (geomShader)
		{
			shaderStageCreateInfos[currentStage++] = {
				.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_GEOMETRY_BIT,
				.module = geomShader,
				.pName  = "main"
			};
		}

		// Create tesselation control shader stage
		if (tescShader)
		{
			shaderStageCreateInfos[currentStage++] = {
				.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				.module = tescShader,
				.pName  = "main"
			};
		}

		// Create tesselation evaluation shader stage
		if (teseShader)
		{
			shaderStageCreateInfos[currentStage++] = {
				.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				.module = teseShader,
				.pName  = "main"
			};
		}

		// Create fragment shader stage
		shaderStageCreateInfos[currentStage] = {
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShader,
			.pName  = "main"
		};

		return shaderStageCreateInfos;
	}

	
	void VulkanComputeShader::create(VkDevice device, const char* computeShaderFile, VkAllocationCallbacks const* pAllocator)
	{
		constexpr auto FILE_MODE = FileModeFlags::eRead | FileModeFlags::eOpenExisting | FileModeFlags::eBinary;

		const auto computeShaderCode = readFile(computeShaderFile, FILE_MODE);
		computeShader = detail::createShaderModule(device, computeShaderCode, pAllocator);
	}

	void VulkanComputeShader::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyShaderModule(device, computeShader, pAllocator);
	}

	auto VulkanComputeShader::getShaderStageCreateInfo() -> VkPipelineShaderStageCreateInfo
	{
		return {
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = computeShader,
			.pName  = "main"
		};
	}


	// namespace detail functions

	auto detail::createShaderModule(
		VkDevice device,
		AxArray<char> const& code,
		VkAllocationCallbacks const* pAllocator) -> VkShaderModule
	{
		VkShaderModuleCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = code.size(),
			.pCode = reinterpret_cast<uint32 const*>(code.data())
		};

		VkShaderModule shaderModule{};
		axVerifyMsg(VK_SUCCESS == vkCreateShaderModule(device, &createInfo, pAllocator, &shaderModule),
			"Failed to create shader module!"
		);

		return shaderModule;
	}

}
