#pragma once
#include <vulkan/vulkan_core.h>

#include "Core/Types.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/Effects/BasicPipeline.h"
#include "Vulkan/Effects/BasicRenderPass.h"
#include "Vulkan/Effects/ScreenPipeline.h"

namespace apex::vk
{
	class VulkanContext;
}

namespace apex {
namespace gfx {

	class ForwardRenderer
	{
	public:
		void initialize(vk::VulkanContext& context);
		void shutdown();

		void onUpdate(Timestep dt);
		void onWindowResize(uint32 width, uint32 height);

	protected:
		void resizeFramebuffers();
		void createSyncObjects(VkDevice device, VkAllocationCallbacks const* pAllocator);
		void allocateCommandBuffers(VkDevice device, VkCommandPool command_pool);
		void prepareGeometry();
		void recordCommandBuffer(VkCommandBuffer command_buffer, uint32 image_index);
		void drawFrame(VkDevice device, VkSwapchainKHR swapchain);


	private:
		// Renderer-specific // TODO: Consider moving to separate struct for renderer
		vk::BasicRenderPass      m_renderPass{}; // TODO: Replace with array
		vk::BasicPipeline        m_pipeline{}; // TODO: Add more pipelines as required

		VkCommandBuffer          m_commandBuffers[vk::VulkanContext::kMaxFramesInFlight]{}; // TODO: Replace with array

		VkSemaphore              m_imageAvailableSemaphores[vk::VulkanContext::kMaxFramesInFlight]{};
		VkSemaphore              m_renderFinishedSemaphores[vk::VulkanContext::kMaxFramesInFlight]{};
		VkFence                  m_inFlightFences[vk::VulkanContext::kMaxFramesInFlight]{};

		// TODO: Move to Mesh/ECS/somewhere with the mesh data
		vk::VulkanBuffer         m_vertexBuffer{}, m_indexBuffer{};


		uint32                   m_currentFrame = 0;

		VkClearValue             m_clearColor{{ 0.f, 0.f, 0.f, 1.f }};

		bool                     m_isFramebufferResized = false;

		vk::VulkanContext*       m_context{};
	};

}
}
