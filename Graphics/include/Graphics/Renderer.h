#pragma once
#include <vulkan/vulkan_core.h>

#include "CommandList.h"
#include "Core/Types.h"

namespace apex::vk
{
	struct VulkanDevice;
	class VulkanContext;
}

namespace apex {
namespace gfx {
	struct Camera;

	struct FrameData
	{
		VkSemaphore     imageAvailableSemaphore{};
		VkSemaphore     renderFinishedSemaphore{};
		VkFence         renderFence{};

		VkCommandPool   commandPool{};
		VkCommandBuffer commandBuffer{};

		CommandList     commandList{};
	};

	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = default;

		virtual void initialize(vk::VulkanContext& context) = 0;
		virtual void shutdown() = 0;

		static constexpr uint32  kMaxFramesInFlight { 1 };

		FrameData& getFrameData(uint32 frame_index) { return m_perFrameData[frame_index]; }

	protected:
		void initializePerFrameData(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator);
		void destroyPerFrameData(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator);
		void createCommandPools(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator);

	private:
		FrameData m_perFrameData[kMaxFramesInFlight]{};
	};

}
}
