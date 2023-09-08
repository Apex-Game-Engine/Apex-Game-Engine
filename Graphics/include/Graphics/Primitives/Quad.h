#pragma once

namespace apex {
namespace gfx {
	class IndexBufferCPU;
	class VertexBufferCPU;

	struct Quad
	{
		static auto getVertexBuffer() -> VertexBufferCPU;
		static auto getIndexBuffer() -> IndexBufferCPU;
		static size_t getNumVertices(); // number of vertices
		static size_t getNumIndices(); // number of indices
	};

}
}
