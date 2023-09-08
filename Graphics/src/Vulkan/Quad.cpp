#include "Graphics/Primitives/Quad.h"

#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/Vertex.h"

namespace apex::gfx
{
	namespace
	{
		constexpr Vertex_P0_C0 vertices[] = {
			{ .position = { -0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } },
			{ .position = {  0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } },
			{ .position = {  0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } },
			{ .position = { -0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } },
		};

		constexpr uint32 indices[] = {
			0, 1, 2,
			2, 3, 0
		};
	}

	auto Quad::getVertexBuffer() -> VertexBufferCPU
	{
		VertexBufferCPU vertexBufferCpu{};
		vertexBufferCpu.create<Vertex_P0_C0>(vertices, getNumVertices());

		return vertexBufferCpu;
	}

	auto Quad::getIndexBuffer() -> IndexBufferCPU
	{
		IndexBufferCPU indexBufferCpu{};
		indexBufferCpu.create(indices, getNumIndices());

		return indexBufferCpu;
	}

	size_t Quad::getNumVertices()
	{
		return std::size(vertices);
	}

	size_t Quad::getNumIndices()
	{
		return std::size(indices);
	}
}
