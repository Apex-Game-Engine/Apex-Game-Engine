#pragma once
#include "Vertex.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Vulkan/VulkanBuffer.h"

namespace apex {
namespace gfx {

	struct IndexBufferGPU
	{
		vk::VulkanBuffer m_buffer;
		size_t           m_count;

		void create(vk::VulkanDevice const& device, IndexBufferCPU const& index_buffer_cpu, VkAllocationCallbacks const* pAllocator);
		void destroy(VkDevice device, VkAllocationCallbacks const* pAllocator);
	};

}
}
