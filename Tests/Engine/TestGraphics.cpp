#include <gtest/gtest.h>
#include "Api.h"
#include "Containers/AxArray.h"

#include "Graphics/Geometry/Vertex.h"
#include "Graphics/Geometry/VertexBufferCPU.h"
#include "Memory/MemoryManager.h"

class TestGraphics : public ::testing::Test
{
public:
	static void SetUpTestSuite()
	{
		apex::mem::MemoryManager::initialize({0, 0});
	}

	static void TearDownTestSuite()
	{
		apex::mem::MemoryManager::shutdown();
	}
};

TEST_F(TestGraphics, TestVertexConvertToFloatPointer)
{
	apex::gfx::Vertex_P0_C0 vertices[] = {
		{ .position = { -0.5f, -0.5f, 0.f }, .color = { 1.f, 0.f, 0.f, 1.f } },
		{ .position = { -0.5f,  0.5f, 0.f }, .color = { 0.f, 1.f, 0.f, 1.f } },
		{ .position = {  0.5f,  0.5f, 0.f }, .color = { 0.f, 0.f, 1.f, 1.f } },
		{ .position = {  0.5f, -0.5f, 0.f }, .color = { 1.f, 1.f, 1.f, 1.f } },
	};

	auto varr = reinterpret_cast<float*>(vertices);

	EXPECT_FLOAT_EQ(varr[0], -0.5f);
	EXPECT_FLOAT_EQ(varr[1], -0.5f);
	EXPECT_FLOAT_EQ(varr[2],  0.0f);
	EXPECT_FLOAT_EQ(varr[3],  1.0f);
	EXPECT_FLOAT_EQ(varr[4],  0.0f);
	EXPECT_FLOAT_EQ(varr[5],  0.0f);
	EXPECT_FLOAT_EQ(varr[6],  1.0f);

	EXPECT_FLOAT_EQ(varr[7], -0.5f);
	EXPECT_FLOAT_EQ(varr[8],  0.5f);
	EXPECT_FLOAT_EQ(varr[9],  0.0f);
	EXPECT_FLOAT_EQ(varr[10],  0.0f);
	EXPECT_FLOAT_EQ(varr[11],  1.0f);
	EXPECT_FLOAT_EQ(varr[12],  0.0f);
	EXPECT_FLOAT_EQ(varr[13],  1.0f);
}

TEST_F(TestGraphics, TestVertexManagedClassAdapter)
{
	apex::gfx::Vertex_P0_C0 vertex { .position = { -0.5f, -0.5f, 0.f }, .color = { 1.f, 0.f, 0.f, 1.f } };

	auto fv = reinterpret_cast<float*>(&vertex);
	EXPECT_FLOAT_EQ(fv[0], -0.5f);
	EXPECT_FLOAT_EQ(fv[1], -0.5f);
	EXPECT_FLOAT_EQ(fv[2],  0.0f);
	EXPECT_FLOAT_EQ(fv[3],  1.0f);
	EXPECT_FLOAT_EQ(fv[4],  0.0f);
	EXPECT_FLOAT_EQ(fv[5],  0.0f);
	EXPECT_FLOAT_EQ(fv[6],  1.0f);
}

TEST_F(TestGraphics, TestVertexAxArrayConvertToFloatAxArray)
{
	apex::AxArray<apex::gfx::Vertex_P0_C0> vertices = {
		{ .position = { -0.5f, -0.5f, 0.f }, .color = { 1.f, 0.f, 0.f, 1.f } },
		{ .position = { -0.5f,  0.5f, 0.f }, .color = { 0.f, 1.f, 0.f, 1.f } },
		{ .position = {  0.5f,  0.5f, 0.f }, .color = { 0.f, 0.f, 1.f, 1.f } },
		{ .position = {  0.5f, -0.5f, 0.f }, .color = { 1.f, 1.f, 1.f, 1.f } },
	};

	using VertexArray = apex::AxArray<apex::gfx::Vertex_P0_C0>;
	static_assert(sizeof(VertexArray::value_type) == 28);

	EXPECT_EQ(sizeof(VertexArray::stored_type), 28);

	apex::AxArrayRef<float> varr { ._data = reinterpret_cast<float*>(vertices.data()), .count = vertices[0].size() * vertices.size() };

	EXPECT_FLOAT_EQ(varr[0], -0.5f);
	EXPECT_FLOAT_EQ(varr[1], -0.5f);
	EXPECT_FLOAT_EQ(varr[2],  0.0f);
	EXPECT_FLOAT_EQ(varr[3],  1.0f);
	EXPECT_FLOAT_EQ(varr[4],  0.0f);
	EXPECT_FLOAT_EQ(varr[5],  0.0f);
	EXPECT_FLOAT_EQ(varr[6],  1.0f);

	EXPECT_FLOAT_EQ(varr[7], -0.5f);
	EXPECT_FLOAT_EQ(varr[8],  0.5f);
	EXPECT_FLOAT_EQ(varr[9],  0.0f);
	EXPECT_FLOAT_EQ(varr[10],  0.0f);
	EXPECT_FLOAT_EQ(varr[11],  1.0f);
	EXPECT_FLOAT_EQ(varr[12],  0.0f);
	EXPECT_FLOAT_EQ(varr[13],  1.0f);
}

TEST_F(TestGraphics, TestVertexBufferCPU)
{
	apex::AxArray<apex::gfx::Vertex_P0_C0> vertices = {
		{ .position = { -0.5f, -0.5f, 0.f }, .color = { 1.f, 0.f, 0.f, 1.f } },
		{ .position = { -0.5f,  0.5f, 0.f }, .color = { 0.f, 1.f, 0.f, 1.f } },
		{ .position = {  0.5f,  0.5f, 0.f }, .color = { 0.f, 0.f, 1.f, 1.f } },
		{ .position = {  0.5f, -0.5f, 0.f }, .color = { 1.f, 1.f, 1.f, 1.f } },
	};

	apex::gfx::VertexBufferCPU vertexBufferCpu{};
	vertexBufferCpu.create(vertices);

	EXPECT_EQ(vertexBufferCpu.vertexSize(), sizeof(apex::gfx::Vertex_P0_C0));
	EXPECT_EQ(vertexBufferCpu.count(), 4);

	EXPECT_EQ(vertexBufferCpu.data()[0], vertices[0].position[0]);
	EXPECT_EQ(vertexBufferCpu.data()[1], vertices[0].position[1]);
	EXPECT_EQ(vertexBufferCpu.data()[2], vertices[0].position[2]);
	EXPECT_EQ(vertexBufferCpu.data()[3], vertices[0].color[0]);
	EXPECT_EQ(vertexBufferCpu.data()[4], vertices[0].color[1]);
	EXPECT_EQ(vertexBufferCpu.data()[5], vertices[0].color[2]);
	EXPECT_EQ(vertexBufferCpu.data()[6], vertices[0].color[3]);

	EXPECT_EQ(vertexBufferCpu.data()[7], vertices[1].position[0]);
	EXPECT_EQ(vertexBufferCpu.data()[8], vertices[1].position[1]);
	EXPECT_EQ(vertexBufferCpu.data()[9], vertices[1].position[2]);
	EXPECT_EQ(vertexBufferCpu.data()[10], vertices[1].color[0]);
	EXPECT_EQ(vertexBufferCpu.data()[11], vertices[1].color[1]);
	EXPECT_EQ(vertexBufferCpu.data()[12], vertices[1].color[2]);
	EXPECT_EQ(vertexBufferCpu.data()[13], vertices[1].color[3]);

	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[7], -0.5f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[8],  0.5f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[9],  0.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[10],  0.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[11],  1.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[12],  0.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[13],  1.0f);


	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[21],  0.5f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[22], -0.5f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[23],  0.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[24],  1.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[25],  1.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[26],  1.0f);
	EXPECT_FLOAT_EQ(vertexBufferCpu.data()[27],  1.0f);
}
