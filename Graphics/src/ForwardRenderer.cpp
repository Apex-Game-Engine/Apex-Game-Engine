#include "Graphics/ForwardRenderer.h"

#include "Core/Asserts.h"
#include "Graphics/Camera.h"
#include "Graphics/CommandList.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Primitives/Quad.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtility.h"
#include "Math/Math.h"
#include "Math/Matrix4x4.h"

namespace apex::gfx {

	void ForwardRenderer::initialize(vk::VulkanContext& context)
	{
		m_context = &context;

		// Create depth buffer
		createDepthBuffer();

		// Create render pass
		//m_renderPass.create(m_context->m_device.logicalDevice, m_context->m_swapchain.surfaceFormat.format, &m_depthImage, VULKAN_NULL_ALLOCATOR);

		// Create swapchain framebuffers
		//m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, &m_depthImage.imageView, VULKAN_NULL_ALLOCATOR);

		// Create descriptor set layouts
		{
			vk::VulkanDescriptorSetLayoutBuilder cameraDescriptorSetLayoutBuilder;
			vk::CameraDescriptorSetLayoutRecipe cameraDescriptorSetLayoutRecipe;
			cameraDescriptorSetLayoutRecipe.build(cameraDescriptorSetLayoutBuilder);
			m_cameraDescriptorSetLayout = cameraDescriptorSetLayoutBuilder.build(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);
		}

		VkDescriptorSetLayout descriptorSetLayouts[] = { m_cameraDescriptorSetLayout.layout };

		VkPushConstantRange pushConstantRanges[] = {
			{
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(math::Matrix4x4)
			}
		};

		// Create pipeline
		createGraphicsPipelineLayout(
			m_context->m_device,
			{ .data = descriptorSetLayouts, .count = std::size(descriptorSetLayouts) },
			{ .data = pushConstantRanges, .count = std::size(pushConstantRanges) },
			VULKAN_NULL_ALLOCATOR);

		vk::VulkanShaderStagesDesc shaderStagesDesc {
			.vertShaderFile = "X:\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.vert.spv",
			.fragShaderFile = "X:\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.frag.spv"
		};

		createGraphicsPipeline(m_context->m_device, shaderStagesDesc, Vertex_P0_C0::getVertexDescription(), m_context->m_swapchain.extent, VULKAN_NULL_ALLOCATOR);

		// Initialize per-frame resources
		initializePerFrameData(m_context->m_device, VULKAN_NULL_ALLOCATOR);

		// Prepare geometry
		prepareGeometry(m_context->m_device, VULKAN_NULL_ALLOCATOR);

		// Create descriptor pool and sets
		createDescriptorPool(m_context->m_device, VULKAN_NULL_ALLOCATOR);

		// Create uniform buffers
		createUniformBuffers(m_context->m_device, VULKAN_NULL_ALLOCATOR);

		// Allocate and populate descriptor sets
		createDescriptorSets(m_context->m_device);
	}

	void ForwardRenderer::shutdown()
	{
		vkDeviceWaitIdle(m_context->m_device.logicalDevice);

		for (uint32 i = 0; i < kMaxFramesInFlight; i++)
		{
			destroyPerFrameData(m_context->m_device, VULKAN_NULL_ALLOCATOR);

			//vmaUnmapMemory(m_context->m_device.vmaAllocator, m_uniformBuffers[i].allocation);
			m_uniformBuffers[i].destroy(m_context->m_device, VULKAN_NULL_ALLOCATOR);
		}

		m_cameraDescriptorSetLayout.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);

		vkDestroyDescriptorPool(m_context->m_device.logicalDevice, m_descriptorPool, VULKAN_NULL_ALLOCATOR);

		//m_pipeline.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);
		vkDestroyPipeline(m_context->m_device.logicalDevice, m_graphicsPipeline, VULKAN_NULL_ALLOCATOR);
		vkDestroyPipelineLayout(m_context->m_device.logicalDevice, m_graphicsPipelineLayout, VULKAN_NULL_ALLOCATOR);

		//m_renderPass.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);

		m_depthImage.destroy(m_context->m_device, VULKAN_NULL_ALLOCATOR);
	}

	void ForwardRenderer::onUpdate(Timestep dt)
	{
		auto& commandList = getCurrentCommandList();
		drawFrame(m_context->m_device, m_context->m_swapchain, commandList);
	}

	void ForwardRenderer::onWindowResize(uint32 /*width*/, uint32 /*height*/)
	{
		resizeFramebuffers();
	}

	auto ForwardRenderer::getCurrentCommandList() -> CommandList&
	{
		return getFrameData(m_currentFrame).commandList;
	}

	auto ForwardRenderer::getContext() -> vk::VulkanContext&
	{
		return *m_context;
	}

	void ForwardRenderer::setActiveCamera(Camera* camera)
	{
		m_activeCamera = camera;
	}

	void ForwardRenderer::stop()
	{
		vkDeviceWaitIdle(m_context->m_device.logicalDevice);
	}

	void ForwardRenderer::resizeFramebuffers()
	{
		// Recreate swapchain framebuffers
		//m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, &m_depthImage.imageView, VULKAN_NULL_ALLOCATOR);
	}

	void ForwardRenderer::prepareGeometry(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		// TODO: Gather all static meshes and create the vertex buffers
		// Create mesh in CPU memory
		//MeshCPU meshCpu = Quad::getMesh();

		// Create mesh in GPU memory
		//m_mesh.create(device, &meshCpu, pAllocator);
	}

	void ForwardRenderer::createGraphicsPipelineLayout(
		vk::VulkanDevice const& device,
		AxArrayRef<VkDescriptorSetLayout> const& descriptor_set_layouts,
		AxArrayRef<VkPushConstantRange> const& push_constant_ranges,
		VkAllocationCallbacks const* pAllocator)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<uint32>(descriptor_set_layouts.count),
			.pSetLayouts = descriptor_set_layouts.data,
			.pushConstantRangeCount = static_cast<uint32>(push_constant_ranges.count),
			.pPushConstantRanges = push_constant_ranges.data
		};

		axVerifyMsg(VK_SUCCESS == vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutCreateInfo, pAllocator, &m_graphicsPipelineLayout),
			"Failed to create graphics pipeline layout!"
		);
	}

	void ForwardRenderer::createGraphicsPipeline(
		vk::VulkanDevice const& device,
		vk::VulkanShaderStagesDesc const& shader_stages_desc,
		gfx::VertexDescription const& vertex_description,
		VkExtent2D swapchain_extent,
		VkAllocationCallbacks const* pAllocator)
	{
		// Set shader stages
		vk::VulkanShaderStages shaderStages;
		shaderStages.create(device.logicalDevice, shader_stages_desc, pAllocator);
		AxArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = shaderStages.getShaderStagesCreateInfos(device.logicalDevice);

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
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &vertex_description.bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<uint32>(vertex_description.attributeDescriptions.size()),
			.pVertexAttributeDescriptions = vertex_description.attributeDescriptions.data()
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
			.width = static_cast<float>(getContext().m_swapchain.extent.width),
			.height = static_cast<float>(getContext().m_swapchain.extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		// Create the initial scissor
		VkRect2D scissor {
			.offset = { .x = 0, .y = 0 },
			.extent = getContext().m_swapchain.extent
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
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};

		// Set multisampling state
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
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f
		};

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

		// Set global color blending state
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
		};

		// Set dynamic rendering info
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.pNext = nullptr,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &getContext().m_swapchain.surfaceFormat.format,
			.depthAttachmentFormat = m_depthImage.format,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
		};

		// Create graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineCreateInfo {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingCreateInfo,
			.stageCount = static_cast<uint32>(shaderStageCreateInfos.size()),
			.pStages = shaderStageCreateInfos.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilStateCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = m_graphicsPipelineLayout,
			.renderPass = VK_NULL_HANDLE,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		axVerifyMsg(VK_SUCCESS == vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, pAllocator, &m_graphicsPipeline),
			"Failed to create graphics pipeline!"
		);
	}

	void ForwardRenderer::createDepthBuffer()
	{
		vk::VulkanImageBuilder depthImageBuilder;
		m_depthImage = depthImageBuilder
			.setExtent(m_context->m_swapchain.extent.width, m_context->m_swapchain.extent.height, 1)
			.setImageType(VK_IMAGE_TYPE_2D)
			.setFormat(VK_FORMAT_D32_SFLOAT)
			.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.setMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.setAllocationRequiredFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.build(m_context->m_device, VULKAN_NULL_ALLOCATOR);
	}

	void ForwardRenderer::createUniformBuffers(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		const auto bufferSize = sizeof(Camera) + sizeof(apex::math::Matrix4x4) * 110;
		uint32 queueFamilyIndices[] = { device.queueFamilyIndices.graphicsFamily.value() };

		for (uint32 i = 0; i < kMaxFramesInFlight; i++)
		{
			m_uniformBuffers[i].create(
				device,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				{ .data = queueFamilyIndices, .count = std::size(queueFamilyIndices) },
				VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				pAllocator);

			/*axVerifyMsg(VK_SUCCESS == vmaMapMemory(device.vmaAllocator, m_uniformBuffers[i].allocation, &m_uniformBuffersMapped[i]),
				"Failed to map uniform buffer memory!"
			);*/
			m_uniformBuffersMapped[i] = m_uniformBuffers[i].getMappedMemory();
		}
	}

	void ForwardRenderer::createDescriptorPool(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		VkDescriptorPoolSize poolSize {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = kMaxFramesInFlight
		};

		VkDescriptorPoolCreateInfo poolCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = kMaxFramesInFlight,
			.poolSizeCount = 1,
			.pPoolSizes = &poolSize,
		};

		axVerifyMsg(VK_SUCCESS == vkCreateDescriptorPool(device.logicalDevice, &poolCreateInfo, pAllocator, &m_descriptorPool),
			"Failed to create descriptor pool!"
		);
	}

	void ForwardRenderer::createDescriptorSets(vk::VulkanDevice const& device)
	{
		AxArray<VkDescriptorSetLayout> layouts(kMaxFramesInFlight);
		layouts.resize(kMaxFramesInFlight, m_cameraDescriptorSetLayout.layout);

		VkDescriptorSetAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = kMaxFramesInFlight,
			.pSetLayouts = layouts.data(),
		};

		axVerifyMsg(VK_SUCCESS == vkAllocateDescriptorSets(device.logicalDevice, &allocateInfo, m_descriptorSets),
			"Failed to allocate descriptor sets!"
		);

		AxArray<VkDescriptorBufferInfo> descriptorBufferInfos(kMaxFramesInFlight * 2);
		AxArray<VkWriteDescriptorSet> descriptorWrites(kMaxFramesInFlight * 2);

		for (uint32 i = 0; i < kMaxFramesInFlight; i++)
		{
			descriptorBufferInfos.append({
				.buffer = m_uniformBuffers[i].buffer,
				.offset = 0,
				.range = sizeof(Camera)
			});

			descriptorWrites.append({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &descriptorBufferInfos.back(),
				.pTexelBufferView = nullptr
			});

			// vkUpdateDescriptorSets(device.logicalDevice, 1, &descriptorWrite, 0, nullptr);

			descriptorBufferInfos.append({
				.buffer = m_uniformBuffers[i].buffer,
				.offset = sizeof(Camera),
				.range = VK_WHOLE_SIZE
			});

			descriptorWrites.append({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &descriptorBufferInfos.back(),
				.pTexelBufferView = nullptr
			});

			// vkUpdateDescriptorSets(device.logicalDevice, 1, &descriptorWrite_model, 0, nullptr);
		}

		vkUpdateDescriptorSets(device.logicalDevice, static_cast<uint32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void ForwardRenderer::recordCommandBuffer(VkCommandBuffer command_buffer, uint32 image_index, gfx::CommandList const& command_list)
	{
		// Begin recording command buffer
		VkCommandBufferBeginInfo commandBufferBeginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = 0,
			.pInheritanceInfo = nullptr // (optional) only for secondary buffers to specify which state to inherit from primary
		};

		axVerifyMsg(VK_SUCCESS == vkBeginCommandBuffer(command_buffer, &commandBufferBeginInfo),
			"Failed to begin recording command buffer!"
		);

		// Transition image layouts to optimal for rendering
		vk::transition_image_layout(
			command_buffer,
			m_context->m_swapchain.images[image_index],
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		vk::transition_image_layout(
			command_buffer,
			m_depthImage.image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);


		// commands begin here . . .
		// Begin render pass
		VkRenderingAttachmentInfo colorAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.pNext = nullptr,
			.imageView = m_context->m_swapchain.imageViews[image_index],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = m_clearColor
		};

		VkRenderingAttachmentInfo depthAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.pNext = nullptr,
			.imageView = m_depthImage.imageView,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue = m_clearDepth
		};

		VkRenderingInfo renderingInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
			.pNext = nullptr,
			.renderArea = { .offset = { 0, 0 }, .extent = m_context->m_swapchain.extent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo,
			.pStencilAttachment = nullptr
		};

		vkCmdBeginRendering(command_buffer, &renderingInfo);

		// Bind graphics pipeline
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		// Set viewport
		VkViewport viewport {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float32>(m_context->m_swapchain.extent.width),
			.height = static_cast<float32>(m_context->m_swapchain.extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		// Set scissor
		VkRect2D scissor {
			.offset = { 0, 0 },
			.extent = m_context->m_swapchain.extent
		};

		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		// Bind descriptor sets
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);

		uint32 uniformIndex = 0;
		for (auto& cmd : getCurrentCommandList().getCommands())
		{
			switch (cmd->type)
			{
			case Command::Type::Draw:
				{
					auto& drawCmd = static_cast<DrawCommand&>(*cmd);
					if (drawCmd.instanceCount == 0)
						continue;

					auto& mesh = *drawCmd.pMesh;

					// Bind vertex buffers
					VkBuffer vertexBuffers[] = {mesh.getVertexBuffer().m_buffer.buffer};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(command_buffer, 0, std::size(vertexBuffers), vertexBuffers, offsets);

					// Bind index buffer
					vkCmdBindIndexBuffer(command_buffer, mesh.getIndexBuffer().m_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

					// Push constants
					vkCmdPushConstants(command_buffer, m_graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &uniformIndex);

					// Submit draw commands
					if (drawCmd.pMesh->getSubMeshCount() == 0)
						vkCmdDrawIndexed(command_buffer, mesh.getIndexBuffer().m_count, drawCmd.instanceCount, 0, 0, 0);
					else
						vkCmdDrawIndexed(command_buffer, mesh.getSubMesh(drawCmd.subMeshIdx).m_indexCount, drawCmd.instanceCount, mesh.getSubMesh(drawCmd.subMeshIdx).m_indexOffset, 0, 0);

					uniformIndex += drawCmd.instanceCount;
				}
				break;
			default:
				break;
			}
		}

		// End render pass
		vkCmdEndRendering(command_buffer);

		vk::transition_image_layout(
			command_buffer,
			m_context->m_swapchain.images[image_index],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		// . . . commands end here
		// End recording command buffer
		axVerifyMsg(VK_SUCCESS == vkEndCommandBuffer(command_buffer),
			"Failed to record command buffer!"
		);
	}

	void ForwardRenderer::updateUniformBuffers()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		Camera* camera = (Camera*)m_uniformBuffersMapped[m_currentFrame];

		if (m_activeCamera)
		{
			camera->view = m_activeCamera->view;
			camera->projection = m_activeCamera->projection;
		}
		else
		{
			camera->view = math::lookAt({ 0.f, 0.f, 3.f }, -math::Vector3::unitZ(), math::Vector3::unitY());

			float32 fov = math::radians(60.f);
			float32 aspect = static_cast<float32>(m_context->m_swapchain.extent.width) / static_cast<float32>(m_context->m_swapchain.extent.height);
			camera->projection = math::perspective(fov, aspect, 0.1f, 100.f);
			camera->projection[1][1] *= -1;
		}

		// TODO: THIS IS FUCKING WRONG! IT WILL OVERFLOW!!!!
		math::Matrix4x4* transform = (math::Matrix4x4*)((uint8*)m_uniformBuffersMapped[m_currentFrame] + sizeof(Camera));

		for (auto& command : getCurrentCommandList().getCommands())
		{
			if (command->type == Command::Type::Draw)
			{
				auto& drawCmd = static_cast<DrawCommand&>(*command);
				*transform = drawCmd.transform;
				transform += 1;
			}
		}
	}

	void ForwardRenderer::drawFrame(vk::VulkanDevice const& device, vk::VulkanSwapchain const& swapchain, CommandList const& command_list)
	{
		FrameData& frameData = getFrameData(m_currentFrame);
		VkCommandPool& commandPool = frameData.commandPool;
		VkCommandBuffer& commandBuffer = frameData.commandBuffer;
		VkSemaphore& imageAvailableSemaphore = frameData.imageAvailableSemaphore;
		VkSemaphore& renderFinishedSemaphore = frameData.renderFinishedSemaphore;
		VkFence& renderFence = frameData.renderFence;

		// Check if framebuffer has been resized

		// Wait for the previous frame to finish
		VkResult result = vkWaitForFences(device.logicalDevice, 1, &renderFence, VK_TRUE, 1'000'000'000 /* ns */);
		axAssertMsg(VK_SUCCESS == result, "Timed out waiting for previous frame to finish!");

		// Acquire an image from the swapchain
		uint32 imageIndex;
		result = vkAcquireNextImageKHR(device.logicalDevice, swapchain.swapchain, 100'000'000'000 /* ns */, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			m_context->handleWindowResize();
			resizeFramebuffers();
			return;
		}
		axAssertMsg(VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result, "Failed to acquire swapchain image!");

		// Reset fence to unsignalled state to begin rendering next frame
		vkResetFences(device.logicalDevice, 1, &renderFence);

		// Udpate uniforms
		updateUniformBuffers();

		// Combine mesh instances into a single draw call
		{
			DrawCommand* pPrevDrawCmd = nullptr;
			for (auto& command : getCurrentCommandList().getCommands())
			{
				if (command->type == Command::Type::Draw)
				{
					auto pCurrentDrawCmd = static_cast<DrawCommand*>(command.get());

					if (pPrevDrawCmd && pCurrentDrawCmd->pMesh == pPrevDrawCmd->pMesh && pCurrentDrawCmd->subMeshIdx == pPrevDrawCmd->subMeshIdx)
					{
						pPrevDrawCmd->instanceCount += pCurrentDrawCmd->instanceCount;
						pCurrentDrawCmd->instanceCount = 0;
					}
					else
					{
						pPrevDrawCmd = pCurrentDrawCmd;
					}
				}
			}
		}
		
		// Record the command buffer for drawing on acquired image
		vkResetCommandPool(m_context->m_device.logicalDevice, commandPool, 0);
		recordCommandBuffer(commandBuffer, imageIndex, command_list);

		// Submit the command buffer
#if USE_OLD_SUBMIT_INFO
  VkSemaphore waitSemaphores[] = { imageAvailableSemaphore }; // semaphores to wait on before execution
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // stage to wait in (corresponds in index to waitSemaphores)

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore }; // semaphores to signal after execution complete

		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = static_cast<uint32>(std::size(waitSemaphores)),
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = static_cast<uint32>(std::size(signalSemaphores)),
			.pSignalSemaphores = signalSemaphores
		};

		axVerifyMsg(VK_SUCCESS == vkQueueSubmit(m_context->m_device.graphicsQueue, 1, &submitInfo, renderFence),
			"Failed to submit draw command buffer!"
		);
#else

		VkSemaphoreSubmitInfo waitSemaphoreInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.semaphore = imageAvailableSemaphore,
			.value = 1,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.deviceIndex = 0,
		};

		VkSemaphoreSubmitInfo signalSemaphoreInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.semaphore = renderFinishedSemaphore,
			.value = 1,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
			.deviceIndex = 0,
		};

		VkCommandBufferSubmitInfo commandBufferInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = commandBuffer,
			.deviceMask = 0,
		};

		VkSubmitInfo2 submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &waitSemaphoreInfo,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &commandBufferInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalSemaphoreInfo,
		};

		axVerifyMsg(VK_SUCCESS == vkQueueSubmit2(m_context->m_device.graphicsQueue, 1, &submitInfo, renderFence),
			"Failed to submit draw command buffer!"
		);
#endif

		// Present the swapchain image to the window surface
		VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderFinishedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain.swapchain,
			.pImageIndices = &imageIndex,
			.pResults = nullptr
		};

		result = vkQueuePresentKHR(m_context->m_device.presentQueue, &presentInfo);

		if (VK_ERROR_OUT_OF_DATE_KHR == result || VK_SUBOPTIMAL_KHR == result)
		{
			m_context->handleWindowResize();
			resizeFramebuffers();
		}
		else
		{
			axAssertMsg(VK_SUCCESS == result, "Failed to present swapchain image!");
		}

		// Advance to next frame
		m_currentFrame = (m_currentFrame + 1) % kMaxFramesInFlight;
	}
}
