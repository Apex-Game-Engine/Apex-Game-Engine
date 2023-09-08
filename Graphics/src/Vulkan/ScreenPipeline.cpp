#include "Graphics/Vulkan/Effects/ScreenPipeline.h"

namespace apex::vk {

	void ScreenPipeline::create(
		VkDevice device,
		VulkanShaderStagesDesc const& shader_stages_desc,
		VkExtent2D swapchain_extent,
		VkRenderPass render_pass,
		VkAllocationCallbacks const* pAllocator)
	{
		VulkanShaderStages shaderStages;
		shaderStages.create(device, shader_stages_desc, pAllocator);
		AxArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = shaderStages.getShaderStagesCreateInfos(device);
		
		// Set up fixed stages of the pipeline
		// Set dynamic state variables
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32>(std::size(dynamicStates)),
			.pDynamicStates = dynamicStates
		};

		// Set vertex input definition
		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		// Set primitive input assembly description
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		// Create initial viewport
		VkViewport viewport {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(swapchain_extent.width),
			.height = static_cast<float>(swapchain_extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		// Create the initial scissor
		VkRect2D scissor {
			.offset = { .x = 0, .y = 0 },
			.extent = swapchain_extent
		};

		// Set initial viewport and scissor
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor
		};

		// Set rasterizer state
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};

		// Set multisampling state (requires enabling a GPU feature)
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		// Set depth and stencil testing state

		// Create color blending state for framebuffer attachment
		VkPipelineColorBlendAttachmentState colorBlendAttachment {
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		// Set global color blend state
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
		};

		// Create descriptor set layout
		createDescriptorSetLayouts(device, pAllocator);

		// Create pipeline layout of uniform values
		createPipelineLayout(device, pAllocator);

		// Create the pipeline
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<uint32>(shaderStageCreateInfos.size()),
			.pStages = shaderStageCreateInfos.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pTessellationState = nullptr,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = nullptr,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipelineLayout,
			.renderPass = render_pass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};

		axVerifyMsg(VK_SUCCESS == vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, pAllocator, &pipeline),
			"Failed to create graphics pipeline!"
		);

		shaderStages.destroy(device, pAllocator);
	}

	void ScreenPipeline::createPipelineLayout(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<uint32>(descriptorSetLayouts.size()),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};

		axVerifyMsg(VK_SUCCESS == vkCreatePipelineLayout(device, &layoutCreateInfo, pAllocator, &pipelineLayout),
			"Failed to create pipeline layout!"
		);
	}

	void ScreenPipeline::createDescriptorSetLayouts(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		descriptorSetLayouts.resize(1);

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
			{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr
			}
		};

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32>(std::size(layoutBindings)),
			.pBindings = layoutBindings
		};

		axVerifyMsg(VK_SUCCESS == vkCreateDescriptorSetLayout(device, &layoutCreateInfo, pAllocator, &descriptorSetLayouts[0]),
			"Failed to create descriptor set layout!"
		);
	}

}