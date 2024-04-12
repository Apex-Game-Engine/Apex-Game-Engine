#include "Graphics/Vulkan/Effects/BasicRenderPass.h"

#include "Core/Asserts.h"
#include "Graphics/Vulkan/VulkanImage.h"

namespace apex::vk {

	void BasicRenderPass::create(
		VkDevice device,
		VkFormat swapchain_image_format,
		VulkanImage const* depth_image,
		VkAllocationCallbacks const* pAllocator)
	{
		AxArray<VkAttachmentDescription> attachments;
		attachments.reserve(2);

		// Create color buffer attachment
		attachments.append(VkAttachmentDescription {
			.format = swapchain_image_format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		});

		// Create attachment references for subpass
		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference depthAttachmentRef;

		if (depth_image)
		{
			// Create depth buffer attachment
			attachments.append(VkAttachmentDescription {
				.format = depth_image->format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			});

			depthAttachmentRef = VkAttachmentReference{
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
		}

		// Create subpass
		VkSubpassDescription subpass {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = nullptr,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = nullptr,
			.pDepthStencilAttachment = depth_image ? &depthAttachmentRef : nullptr,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = nullptr,
		};

		// Define subpass dependencies
		AxArray<VkSubpassDependency> dependencies;
		dependencies.reserve(2);
		dependencies.append(VkSubpassDependency {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		});
		dependencies.append(VkSubpassDependency {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		});

		// Create render pass
		VkRenderPassCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = static_cast<uint32>(dependencies.size()),
			.pDependencies = dependencies.data(),
		};

		axVerifyMsg(VK_SUCCESS == vkCreateRenderPass(device, &createInfo, pAllocator, &renderPass),
			"Failed to create render pass"
		);
	}

	void BasicRenderPass::destroy(VkDevice device, VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyRenderPass(device, renderPass, pAllocator);
	}

}
