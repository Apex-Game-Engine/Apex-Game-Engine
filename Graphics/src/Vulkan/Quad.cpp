#include "Graphics/Primitives/Quad.h"

#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/Vertex.h"

namespace apex::gfx
{
	namespace mesh_quad
	{
		constexpr Vertex_P0_C0 vertices[] = {
			{ .position = { -0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 0 (bottom-left)
			{ .position = {  0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 1 (bottom-right)
			{ .position = {  0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 2 (top-right)
			{ .position = { -0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3 (top-left)
		};

		constexpr uint32 indices[] = {
			0, 1, 2,
			2, 3, 0
		};
	}

	auto Quad::getMesh() -> MeshCPU
	{
		MeshCPU mesh{};
		mesh.m_vertexBufferCPU.create<Vertex_P0_C0>(mesh_quad::vertices, getVertexCount());
		mesh.m_indexBufferCPU.create(mesh_quad::indices, getIndexCount());

		return mesh;
	}
	
	size_t Quad::getVertexCount()
	{
		return std::size(mesh_quad::vertices);
	}

	size_t Quad::getIndexCount()
	{
		return std::size(mesh_quad::indices);
	}

	auto Quad::getVertexBuffer() -> VertexBufferCPU
	{
		VertexBufferCPU vertexBufferCpu{};
		vertexBufferCpu.create<Vertex_P0_C0>(mesh_quad::vertices, getVertexCount());

		return vertexBufferCpu;
	}

	auto Quad::getIndexBuffer() -> IndexBufferCPU
	{
		IndexBufferCPU indexBufferCpu{};
		indexBufferCpu.create(mesh_quad::indices, getIndexCount());

		return indexBufferCpu;
	}


	namespace mesh_pyramid
	{
		constexpr Vertex_P0_C0 vertices[] = {
			{ .position = {  0.0f,  0.5f,  0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 0 (top)
			{ .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } }, // 1 (back-left)
			{ .position = {  0.5f, -0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }, // 2 (back-right)
			{ .position = {  0.5f, -0.5f,  0.5f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 3 (front-right)
			{ .position = { -0.5f, -0.5f,  0.5f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 4 (front-left)
		};

		constexpr uint32 indices[] = {
			0, 2, 1,
			0, 3, 2,
			0, 4, 3,
			0, 1, 4,
			1, 2, 3,
			1, 3, 4
		};
		
	}

	auto Pyramid::getMesh() -> MeshCPU
	{
		MeshCPU mesh{};
		mesh.m_vertexBufferCPU.create<Vertex_P0_C0>(mesh_pyramid::vertices, getVertexCount());
		mesh.m_indexBufferCPU.create(mesh_pyramid::indices, getIndexCount());

		return mesh;
	}

	size_t Pyramid::getVertexCount()
	{
		return std::size(mesh_pyramid::vertices);
	}

	size_t Pyramid::getIndexCount()
	{
		return std::size(mesh_pyramid::indices);
	}

	auto Pyramid::getVertexBuffer() -> VertexBufferCPU
	{
		VertexBufferCPU vertexBufferCpu{};
		vertexBufferCpu.create<Vertex_P0_C0>(mesh_pyramid::vertices, getVertexCount());

		return vertexBufferCpu;
	}

	auto Pyramid::getIndexBuffer() -> IndexBufferCPU
	{
		IndexBufferCPU indexBufferCpu{};
		indexBufferCpu.create(mesh_pyramid::indices, getIndexCount());

		return indexBufferCpu;
	}
}
