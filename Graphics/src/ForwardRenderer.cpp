#include "Graphics/ForwardRenderer.h"

#include "Core/Asserts.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Primitives/Quad.h"
#include "Graphics/Vulkan/VulkanContext.h"

namespace apex::gfx {

	void ForwardRenderer::initialize(vk::VulkanContext& context)
	{
		m_context = &context;

		// Create render pass
		m_renderPass.create(m_context->m_device.logicalDevice, m_context->m_swapchain.surfaceFormat.format, nullptr);

		// Create swapchain framebuffers
		m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, nullptr);

		// Create pipeline
		vk::VulkanShaderStagesDesc shaderStagesDesc {
			.vertShaderFile = "D:\\Repos\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.vert.spv",
			.fragShaderFile = "D:\\Repos\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\basic.frag.spv"
		};
		m_pipeline.create(m_context->m_device.logicalDevice, shaderStagesDesc, m_context->m_swapchain.extent, m_renderPass.renderPass, nullptr);

		// Create sync objects
		createSyncObjects(m_context->m_device.logicalDevice, nullptr);

		// Allocate command buffers
		allocateCommandBuffers(m_context->m_device.logicalDevice, m_context->m_device.commandPool);

		// Prepare geometry
		prepareGeometry();
	}

	void ForwardRenderer::shutdown()
	{
		vkDeviceWaitIdle(m_context->m_device.logicalDevice);

		m_vertexBuffer.destroy(m_context->m_device, nullptr);
		m_indexBuffer.destroy(m_context->m_device, nullptr);

		for (uint32 frame = 0; frame < vk::VulkanContext::kMaxFramesInFlight; frame++)
		{
			vkFreeCommandBuffers(m_context->m_device.logicalDevice, m_context->m_device.commandPool, 1, &m_commandBuffers[frame]);

			vkDestroySemaphore(m_context->m_device.logicalDevice, m_imageAvailableSemaphores[frame], nullptr);
			vkDestroySemaphore(m_context->m_device.logicalDevice, m_renderFinishedSemaphores[frame], nullptr);
			vkDestroyFence(m_context->m_device.logicalDevice, m_inFlightFences[frame], nullptr);
		}

		m_pipeline.destroy(m_context->m_device.logicalDevice, nullptr);

		m_renderPass.destroy(m_context->m_device.logicalDevice, nullptr);
	}

	void ForwardRenderer::onUpdate(Timestep dt)
	{
		drawFrame(m_context->m_device.logicalDevice, m_context->m_swapchain.swapchain);
	}

	void ForwardRenderer::onWindowResize(uint32 /*width*/, uint32 /*height*/)
	{
		resizeFramebuffers();
	}

	void ForwardRenderer::resizeFramebuffers()
	{
		// Recreate swapchain framebuffers
		m_context->m_swapchain.createFramebuffers(m_context->m_device.logicalDevice, m_renderPass.renderPass, nullptr);
	}

	void ForwardRenderer::createSyncObjects(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		VkFenceCreateInfo fenceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (uint32 frame = 0; frame < vk::VulkanContext::kMaxFramesInFlight; frame++)
		{
			axVerifyMsg(
				VK_SUCCESS == vkCreateSemaphore(device, &semaphoreCreateInfo, pAllocator, &m_imageAvailableSemaphores[frame]) &&
				VK_SUCCESS == vkCreateSemaphore(device, &semaphoreCreateInfo, pAllocator, &m_renderFinishedSemaphores[frame]),
				"Failed to create semaphore!"
			);

			axVerifyMsg(VK_SUCCESS == vkCreateFence(device, &fenceCreateInfo, pAllocator, &m_inFlightFences[frame]),
				"Failed to create fence!"
			);
		}
	}

	void ForwardRenderer::allocateCommandBuffers(VkDevice device, VkCommandPool command_pool)
	{
		VkCommandBufferAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = vk::VulkanContext::kMaxFramesInFlight
		};

		axVerifyMsg(VK_SUCCESS == vkAllocateCommandBuffers(device, &allocateInfo, m_commandBuffers),
			"Failed to allocate command buffers!"
		);
	}

	void ForwardRenderer::prepareGeometry()
	{
		// TODO: Gather all static meshes and create the vertex buffers
		// Create buffers in CPU memory
		VertexBufferCPU vertexBufferCpu = Quad::getVertexBuffer();
		IndexBufferCPU indexBufferCpu = Quad::getIndexBuffer();

		// Transfer buffers to GPU memory
		vk::VulkanBuffer::CreateVertexBufferGPU(m_vertexBuffer, m_context->m_device, vertexBufferCpu, nullptr);
		vk::VulkanBuffer::CreateIndexBufferGPU(m_indexBuffer, m_context->m_device, indexBufferCpu, nullptr);
	}

	void ForwardRenderer::recordCommandBuffer(VkCommandBuffer command_buffer, uint32 image_index)
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

		// Bind vertex buffers
		VkBuffer vertexBuffers[] = { m_vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, std::size(vertexBuffers), vertexBuffers, offsets);

		// Bind index buffer
		vkCmdBindIndexBuffer(command_buffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Submit draw commands
		// TODO: Remove hardcoded vertex count; retrieve from the mesh/buffer
		vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

		// End render pass
		vkCmdEndRenderPass(command_buffer);

		// . . . commands end here
		// End recording command buffer
		axVerifyMsg(VK_SUCCESS == vkEndCommandBuffer(command_buffer),
			"Failed to record command buffer!"
		);
	}

	void ForwardRenderer::drawFrame(VkDevice device, VkSwapchainKHR swapchain)
	{
		VkCommandBuffer& commandBuffer = m_commandBuffers[m_currentFrame];
		VkSemaphore& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
		VkSemaphore& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];
		VkFence& inFlightFence = m_inFlightFences[m_currentFrame];

		// Check if framebuffer has been resized

		// Wait for the previous frame to finish
		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, constants::uint64_MAX);

		// Acquire an image from the swapchain
		uint32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapchain, constants::uint64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			m_context->handleWindowResize();
			resizeFramebuffers();
			return;
		}
		axAssertMsg(VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result, "Failed to acquire swapchain image!");

		// Reset fence to unsignalled state to begin rendering next frame
		vkResetFences(device, 1, &inFlightFence);

		// Record the command buffer for drawing on acquired image
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex);

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
			.pSwapchains = &swapchain,
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
		m_currentFrame = (m_currentFrame + 1) % vk::VulkanContext::kMaxFramesInFlight;
	}
}
