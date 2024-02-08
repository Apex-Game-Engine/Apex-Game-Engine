#include "Graphics/Renderer.h"

#include "Core/Asserts.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanDevice.h"

namespace apex::gfx
{
	void Renderer::initializePerFrameData(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		VkFenceCreateInfo fenceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		createCommandPools(device, pAllocator);

		for (auto& frameData : m_perFrameData)
		{
			axVerifyMsg(
				VK_SUCCESS == vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, pAllocator, &frameData.imageAvailableSemaphore) &&
				VK_SUCCESS == vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, pAllocator, &frameData.renderFinishedSemaphore),
				"Failed to create semaphore!"
			);

			axVerifyMsg(VK_SUCCESS == vkCreateFence(device.logicalDevice, &fenceCreateInfo, pAllocator, &frameData.inFlightFence),
			            "Failed to create fence!"
			);

			VkCommandBufferAllocateInfo allocateInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = frameData.commandPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			axVerifyMsg(VK_SUCCESS == vkAllocateCommandBuffers(device.logicalDevice, &allocateInfo, &frameData.commandBuffer),
				"Failed to allocate command buffers!"
			);
		}
	}

	void Renderer::destroyPerFrameData(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator)
	{
		for (auto& frameData : m_perFrameData)
		{
			vkDestroySemaphore(device.logicalDevice, frameData.imageAvailableSemaphore, pAllocator);
			vkDestroySemaphore(device.logicalDevice, frameData.renderFinishedSemaphore, pAllocator);
			vkDestroyFence(device.logicalDevice, frameData.inFlightFence, pAllocator);
			vkFreeCommandBuffers(device.logicalDevice, frameData.commandPool, 1, &frameData.commandBuffer);
			vkDestroyCommandPool(device.logicalDevice, frameData.commandPool, pAllocator);
		}
	}

	void Renderer::createCommandPools(vk::VulkanDevice& device, VkAllocationCallbacks const* pAllocator)
	{
		VkCommandPoolCreateInfo graphicsPoolCreateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value()
		};

		for (auto& frameData : m_perFrameData)
		{
			axVerifyMsg(VK_SUCCESS == vkCreateCommandPool(device.logicalDevice, &graphicsPoolCreateInfo, pAllocator, &frameData.commandPool),
				"Failed to create graphics command pool!"
			);
		}
	}
}
