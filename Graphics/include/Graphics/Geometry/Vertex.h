﻿#pragma once
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Memory/AxManagedClass.h"

#include <array>
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"

namespace apex {
namespace gfx {

	enum class AttributeType
	{
		eFloat,
		eVec2,
		eVec3,
		eVec4,
	};

	enum class VertexAttribute
	{
		ePosition,
		eTexCoord,
		eNormal,
		eTangent,
		eJointWeight,
		eJointIndex,
		eColor
	};

	constexpr auto getAttributeType(VertexAttribute attr) -> AttributeType
	{
		switch (attr)
		{
		case VertexAttribute::ePosition:    return AttributeType::eVec3;
		case VertexAttribute::eTexCoord:    return AttributeType::eVec2;
		case VertexAttribute::eNormal:      return AttributeType::eVec3;
		case VertexAttribute::eTangent:     return AttributeType::eVec3;
		case VertexAttribute::eJointWeight: return AttributeType::eVec4;
		case VertexAttribute::eJointIndex:  return AttributeType::eFloat;
		case VertexAttribute::eColor:       return AttributeType::eVec4;
		}
	}

	constexpr auto getAttributeTypeSize(AttributeType type) -> size_t
	{
		switch (type)
		{
		case AttributeType::eFloat: return sizeof(float);
		case AttributeType::eVec2: return sizeof(math::Vector2);
		case AttributeType::eVec3: return sizeof(math::Vector3);
		case AttributeType::eVec4: return sizeof(math::Vector4);
		}
	}

	constexpr auto convertToVulkanAttributeFormat(AttributeType type) -> VkFormat
	{
		switch (type)
		{
		case AttributeType::eFloat: return VK_FORMAT_R32_SFLOAT;
		case AttributeType::eVec2:  return VK_FORMAT_R32G32_SFLOAT;
		case AttributeType::eVec3:  return VK_FORMAT_R32G32B32_SFLOAT;
		case AttributeType::eVec4:  return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}

	struct VertexInfo
	{
		AxArrayRef<VertexAttribute const> attributes;
		size_t                 stride;
	};

	template <VertexAttribute... Attrs>
	struct Vertex : public apex::AxManagedClass
	{
		constexpr Vertex() = default;

		static_assert(sizeof...(Attrs) > 0, "Cannot create vertex without any attributes!");

		static constexpr std::array<VertexAttribute, sizeof...(Attrs)> attrs { Attrs... };
		static constexpr auto size() -> size_t
		{
			return (getAttributeTypeSize(getAttributeType(Attrs)) + ...);
		}

		static constexpr auto getBindingDescription() -> VkVertexInputBindingDescription
		{
			return VkVertexInputBindingDescription {
				.binding = 0,
				.stride = size(),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			};
		}

		static constexpr auto getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, sizeof...(Attrs)> attributeDescriptions{};

			uint32 offset = 0;
			for (size_t i = 0; i < sizeof...(Attrs); i++)
			{
				attributeDescriptions[i] = VkVertexInputAttributeDescription {
					.location = static_cast<uint32>(i),
					.binding = 0,
					.format = convertToVulkanAttributeFormat(getAttributeType(attrs[i])),
					.offset = offset
				};
				offset += getAttributeTypeSize(getAttributeType(attrs[i]));
			}

			return attributeDescriptions;
		}

		static constexpr auto getVertexInfo()
		{
			return VertexInfo {
				.attributes = { attrs.data(), attrs.size() },
				.stride = size()
			};
		}
	};

	struct Vertex_P0_C0 : Vertex<VertexAttribute::ePosition,
									VertexAttribute::eColor>
	{
		math::Vector3 position;
		math::Vector4 color;
	};

	static_assert(Vertex_P0_C0::attrs[0] == VertexAttribute::ePosition);
	static_assert(Vertex_P0_C0::attrs[1] == VertexAttribute::eColor);
	static_assert(Vertex_P0_C0::size() == sizeof(Vertex_P0_C0));

	struct Vertex_P0_TC0 : Vertex<VertexAttribute::ePosition,
									VertexAttribute::eTexCoord>
	{
		math::Vector3 position;
		union
		{
			math::Vector2 uv;
			math::Vector2 texcoord;
		};
	};

	static_assert(Vertex_P0_TC0::attrs[0] == VertexAttribute::ePosition);
	static_assert(Vertex_P0_TC0::attrs[1] == VertexAttribute::eTexCoord);
	static_assert(Vertex_P0_TC0::size() == sizeof(Vertex_P0_TC0));

}
}
