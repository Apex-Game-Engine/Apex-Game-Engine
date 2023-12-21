#pragma once
#include <vulkan/vulkan_core.h>

#include "CommandList.h"
#include "Core/Types.h"
#include "Geometry/Mesh.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/Effects/BasicPipeline.h"
#include "Vulkan/Effects/BasicRenderPass.h"
#include "Vulkan/Effects/CameraDescriptorSetLayout.h"
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

		auto getCurrentCommandList() -> CommandList&;
		auto getContext() -> vk::VulkanContext&;

	protected:
		void resizeFramebuffers();
		void createSyncObjects(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
		void allocateCommandBuffers(vk::VulkanDevice const& device, VkCommandPool command_pool);
		void prepareGeometry(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
		void createUniformBuffers(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
		void createDescriptorPool(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
		void createDescriptorSets(vk::VulkanDevice const& device);

		// per frame commands
		void recordCommandBuffer(VkCommandBuffer command_buffer, uint32 image_index, gfx::CommandList const& command_list);
		void updateUniformBuffers();
		void drawFrame(vk::VulkanDevice const& device, vk::VulkanSwapchain const& swapchain, CommandList const& command_list);

		static constexpr uint32  kMaxFramesInFlight { 1 };

	private:
		// Renderer-specific // TODO: Consider moving to separate struct for renderer
		vk::BasicRenderPass            m_renderPass{}; // TODO: Add more render passes as required
		vk::BasicPipeline              m_pipeline{}; // TODO: Add more pipelines as required

		VkDescriptorPool               m_descriptorPool{};
		vk::CameraDescriptorSetLayout  m_cameraDescriptorSetLayout{}; // TODO: Add more descriptor set layouts as required

		VkDescriptorSet                m_descriptorSets[kMaxFramesInFlight]{}; // TODO: Add more as required

		vk::VulkanBuffer               m_uniformBuffers[kMaxFramesInFlight]{};
		void*                          m_uniformBuffersMapped[kMaxFramesInFlight]{};

		VkCommandBuffer                m_commandBuffers[kMaxFramesInFlight]{}; // TODO: Replace with array

		VkSemaphore                    m_imageAvailableSemaphores[kMaxFramesInFlight]{};
		VkSemaphore                    m_renderFinishedSemaphores[kMaxFramesInFlight]{};
		VkFence                        m_inFlightFences[kMaxFramesInFlight]{};

		CommandList                    m_commandLists[kMaxFramesInFlight]{};

		uint32                         m_currentFrame = 0;

		VkClearValue                   m_clearColor{{ 0.f, 0.f, 0.f, 1.f }};

		bool                           m_isFramebufferResized = false;

		vk::VulkanContext*             m_context{};
	};

}
}
