﻿#pragma once
#include "Vertex.h"
#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Vulkan/VulkanBuffer.h"

namespace apex {
namespace gfx {

	struct VertexBufferGPU
	{
		vk::VulkanBuffer m_buffer;
		VertexInfo       m_vertexInfo;
		size_t           m_count{};

		void createMapped(vk::VulkanDevice const& device, VertexInfo const& vertex_info, size_t count, VkAllocationCallbacks const* pAllocator);
		void create(vk::VulkanDevice const& device, VertexBufferCPU const& vertex_buffer_cpu, VkAllocationCallbacks const* pAllocator);
		void destroy(vk::VulkanDevice const& device, VkAllocationCallbacks const* pAllocator);
	};

}
}
