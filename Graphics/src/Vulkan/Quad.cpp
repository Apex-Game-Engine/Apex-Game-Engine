#include "Graphics/Primitives/Quad.h"

#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Graphics/Geometry/IndexBufferCPU.h"
#include "Graphics/Geometry/Vertex.h"

namespace apex::gfx
{
	namespace mesh_quad
	{
		static Vertex_P0_C0 vertices[] = {
			{ .position = { -0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 0 (bottom-left)
			{ .position = {  0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 1 (bottom-right)
			{ .position = {  0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 2 (top-right)
			{ .position = { -0.5f,  0.5f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3 (top-left)
		};

		static uint32 indices[] = {
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


	namespace mesh_cube
	{
		static Vertex_P0_C0 vertices[] = {
			{ .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 0 (front-bottom-left)
			{ .position = {  0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 1 (front-bottom-right)
			{ .position = {  0.5f,  0.5f, -0.5f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 2 (front-top-right)
			{ .position = { -0.5f,  0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3 (front-top-left)

			{ .position = { -0.5f, -0.5f,  0.5f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 4 (back-bottom-left)
			{ .position = {  0.5f, -0.5f,  0.5f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 5 (back-bottom-right)
			{ .position = {  0.5f,  0.5f,  0.5f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 6 (back-top-right)
			{ .position = { -0.5f,  0.5f,  0.5f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 7 (back-top-left)
		};

		static uint32 indices[] = {
			0, 1, 2,
			2, 3, 0,

			1, 5, 6,
			6, 2, 1,

			5, 4, 7,
			7, 6, 5,

			4, 0, 3,
			3, 7, 4,

			3, 2, 6,
			6, 7, 3,

			4, 5, 1,
			1, 0, 4
		};
	}

	auto Cube::getMesh() -> MeshCPU
	{
		MeshCPU mesh{};
		mesh.m_vertexBufferCPU.create<Vertex_P0_C0>(mesh_cube::vertices, getVertexCount());
		mesh.m_indexBufferCPU.create(mesh_cube::indices, getIndexCount());

		return mesh;
	}

	size_t Cube::getVertexCount()
	{
		return std::size(mesh_cube::vertices);
	}

	size_t Cube::getIndexCount()
	{
		return std::size(mesh_cube::indices);
	}

	auto Cube::getVertexBuffer() -> VertexBufferCPU
	{
		VertexBufferCPU vertexBufferCpu{};
		vertexBufferCpu.create<Vertex_P0_C0>(mesh_cube::vertices, getVertexCount());

		return vertexBufferCpu;
	}

	auto Cube::getIndexBuffer() -> IndexBufferCPU
	{
		IndexBufferCPU indexBufferCpu{};
		indexBufferCpu.create(mesh_cube::indices, getIndexCount());

		return indexBufferCpu;
	}


	namespace mesh_pyramid
	{
		static Vertex_P0_C0 vertices[] = {
			{ .position = {  0.0f,  0.5f,  0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 0 (top)
			{ .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } }, // 1 (back-left)
			{ .position = {  0.5f, -0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }, // 2 (back-right)
			{ .position = {  0.5f, -0.5f,  0.5f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 3 (front-right)
			{ .position = { -0.5f, -0.5f,  0.5f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 4 (front-left)
		};

		static uint32 indices[] = {
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
