#include "Graphics/ForwardRenderer.h"

#include "Core/Asserts.h"
#include "Graphics/Camera.h"
#include "Graphics/CommandList.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Primitives/Quad.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Math/Math.h"
#include "Math/Matrix4x4.h"

namespace apex::gfx {

#define VULKAN_NULL_ALLOCATOR (VkAllocationCallbacks*)nullptr

	void ForwardRenderer::initialize(vk::VulkanContext& context)
	{
		m_context = &context;

		// Create render pass
		m_renderPass.create(m_context->m_device.logicalDevice, m_context->m_swapchain.surfaceFormat.format, VULKAN_NULL_ALLOCATOR);

		// Create swapchain framebuffers
		m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, VULKAN_NULL_ALLOCATOR);

		// Create descriptor set layouts
		m_cameraDescriptorSetLayout.create(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);

		VkDescriptorSetLayout descriptorSetLayouts[] = { m_cameraDescriptorSetLayout.layout };

		// Create pipeline
		vk::VulkanShaderStagesDesc shaderStagesDesc {
			.vertShaderFile = "D:\\Repos\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.vert.spv",
			.fragShaderFile = "D:\\Repos\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.frag.spv"
		};
		m_pipeline.create(
			m_context->m_device.logicalDevice,
			shaderStagesDesc,
		    { .data = descriptorSetLayouts, .count = std::size(descriptorSetLayouts) },
			m_context->m_swapchain.extent,
			m_renderPass.renderPass,
			VULKAN_NULL_ALLOCATOR);

		// Create sync objects
		createSyncObjects(m_context->m_device, VULKAN_NULL_ALLOCATOR);

		// Allocate command buffers
		allocateCommandBuffers(m_context->m_device, m_context->m_device.commandPool);

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
			vkFreeCommandBuffers(m_context->m_device.logicalDevice, m_context->m_device.commandPool, 1, &m_commandBuffers[i]);

			vkDestroySemaphore(m_context->m_device.logicalDevice, m_imageAvailableSemaphores[i], VULKAN_NULL_ALLOCATOR);
			vkDestroySemaphore(m_context->m_device.logicalDevice, m_renderFinishedSemaphores[i], VULKAN_NULL_ALLOCATOR);
			vkDestroyFence(m_context->m_device.logicalDevice, m_inFlightFences[i], VULKAN_NULL_ALLOCATOR);

			m_uniformBuffers[i].destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);
		}

		m_cameraDescriptorSetLayout.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);

		vkDestroyDescriptorPool(m_context->m_device.logicalDevice, m_descriptorPool, VULKAN_NULL_ALLOCATOR);

		m_pipeline.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);

		m_renderPass.destroy(m_context->m_device.logicalDevice, VULKAN_NULL_ALLOCATOR);
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
		return m_commandLists[m_currentFrame];
	}

	auto ForwardRenderer::getContext() -> vk::VulkanContext&
	{
		return *m_context;
	}

	void ForwardRenderer::resizeFramebuffers()
	{
		// Recreate swapchain framebuffers
		m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, VULKAN_NULL_ALLOCATOR);
	}

	void ForwardRenderer::createSyncObjects(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		VkFenceCreateInfo fenceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (uint32 frame = 0; frame < kMaxFramesInFlight; frame++)
		{
			axVerifyMsg(
				VK_SUCCESS == vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, pAllocator, &m_imageAvailableSemaphores[frame]) &&
				VK_SUCCESS == vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, pAllocator, &m_renderFinishedSemaphores[frame]),
				"Failed to create semaphore!"
			);

			axVerifyMsg(VK_SUCCESS == vkCreateFence(device.logicalDevice, &fenceCreateInfo, pAllocator, &m_inFlightFences[frame]),
				"Failed to create fence!"
			);
		}
	}

	void ForwardRenderer::allocateCommandBuffers(vk::VulkanDevice const& device, VkCommandPool command_pool)
	{
		VkCommandBufferAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = kMaxFramesInFlight
		};

		axVerifyMsg(VK_SUCCESS == vkAllocateCommandBuffers(device.logicalDevice, &allocateInfo, m_commandBuffers),
			"Failed to allocate command buffers!"
		);
	}

	void ForwardRenderer::prepareGeometry(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		// TODO: Gather all static meshes and create the vertex buffers
		// Create mesh in CPU memory
		//MeshCPU meshCpu = Quad::getMesh();

		// Create mesh in GPU memory
		//m_mesh.create(device, &meshCpu, pAllocator);
	}

	void ForwardRenderer::createUniformBuffers(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator)
	{
		const auto bufferSize = sizeof(Camera);
		uint32 queueFamilyIndices[] = { device.queueFamilyIndices.graphicsFamily.value() };

		for (uint32 i = 0; i < kMaxFramesInFlight; i++)
		{
			m_uniformBuffers[i].create(
				device,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				{ .data = queueFamilyIndices, .count = std::size(queueFamilyIndices) },
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				pAllocator);

			axVerifyMsg(VK_SUCCESS == vkMapMemory(device.logicalDevice, m_uniformBuffers[i].memory, 0, bufferSize, 0, &m_uniformBuffersMapped[i]),
				"Failed to map uniform buffer memory!"
			);
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

		for (uint32 i = 0; i < kMaxFramesInFlight; i++)
		{
			VkDescriptorBufferInfo bufferInfo {
				.buffer = m_uniformBuffers[i].buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};

			VkWriteDescriptorSet descriptorWrite {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &bufferInfo,
				.pTexelBufferView = nullptr
			};

			vkUpdateDescriptorSets(device.logicalDevice, 1, &descriptorWrite, 0, nullptr);
		}
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

		// commands begin here . . .
		// Begin render pass
		VkRenderPassBeginInfo renderPassBeginInfo {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_renderPass.renderPass,
			.framebuffer = m_context->m_swapchain.framebuffers[image_index],
			.renderArea = {
				.offset = { 0, 0 },
				.extent = m_context->m_swapchain.extent
			},
			.clearValueCount = 1,
			.pClearValues = &m_clearColor
		};

		vkCmdBeginRenderPass(command_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind graphics pipeline
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipeline);

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
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);

		for (auto& cmd : m_commandLists[m_currentFrame].getCommands())
		{
			switch (cmd->type)
			{
			case Command::Type::Draw:
				{
					auto& drawCmd = static_cast<DrawCommand&>(*cmd);
					auto& mesh = *drawCmd.pMesh;

					// Bind vertex buffers
					VkBuffer vertexBuffers[] = {mesh.m_vertexBuffer.m_buffer.buffer};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(command_buffer, 0, std::size(vertexBuffers), vertexBuffers, offsets);

					// Bind index buffer
					vkCmdBindIndexBuffer(command_buffer, mesh.m_indexBuffer.m_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

					// Submit draw commands
					vkCmdDrawIndexed(command_buffer, mesh.m_indexBuffer.m_count, 1, 0, 0, 0);
				}
				break;
			default:
				break;
			}
		}

		// End render pass
		vkCmdEndRenderPass(command_buffer);

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
		camera->model = math::rotateY(math::Matrix4x4::identity(), time * math::radians(90.f));
		camera->model = math::rotateX(camera->model, time * math::radians(-60.f));
		camera->view = math::lookAt({ 0.f, 0.f, 3.f }, -math::Vector3::unitZ(), math::Vector3::unitY());

		float32 fov = math::radians(60.f);
		float32 aspect = static_cast<float32>(m_context->m_swapchain.extent.width) / static_cast<float32>(m_context->m_swapchain.extent.height);
		camera->projection = math::perspective(fov, aspect, 0.1f, 100.f);
		camera->projection[1][1] *= -1;
	}

	void ForwardRenderer::drawFrame(vk::VulkanDevice const& device, vk::VulkanSwapchain const& swapchain, CommandList const& command_list)
	{
		VkCommandBuffer& commandBuffer = m_commandBuffers[m_currentFrame];
		VkSemaphore& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
		VkSemaphore& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];
		VkFence& inFlightFence = m_inFlightFences[m_currentFrame];

		// Check if framebuffer has been resized

		// Wait for the previous frame to finish
		vkWaitForFences(device.logicalDevice, 1, &inFlightFence, VK_TRUE, constants::uint64_MAX);

		// Acquire an image from the swapchain
		uint32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(device.logicalDevice, swapchain.swapchain, constants::uint64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			m_context->handleWindowResize();
			resizeFramebuffers();
			return;
		}
		axAssertMsg(VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result, "Failed to acquire swapchain image!");

		// Reset fence to unsignalled state to begin rendering next frame
		vkResetFences(device.logicalDevice, 1, &inFlightFence);

		// Record the command buffer for drawing on acquired image
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex, command_list);

		// Udpate uniforms
		updateUniformBuffers();

		// Submit the command buffer
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

		axAssertMsg(VK_SUCCESS == vkQueueSubmit(m_context->m_device.graphicsQueue, 1, &submitInfo, inFlightFence),
			"Failed to submit draw command buffer!"
		);

		// Present the swapchain image to the window surface
		VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = static_cast<uint32>(std::size(signalSemaphores)),
			.pWaitSemaphores = signalSemaphores,
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
