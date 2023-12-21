#pragma once
#include "Graphics/Geometry/Mesh.h"

namespace apex {
namespace gfx {
	class IndexBufferCPU;
	class VertexBufferCPU;

	struct Quad
	{
		static auto getMesh() -> MeshCPU;
		static size_t getVertexCount(); // number of vertices
		static size_t getIndexCount(); // number of indices

	protected:
		static auto getVertexBuffer() -> VertexBufferCPU;
		static auto getIndexBuffer() -> IndexBufferCPU;
	};

	struct Pyramid
	{
		static auto getMesh() -> MeshCPU;
		static size_t getVertexCount(); // number of vertices
		static size_t getIndexCount(); // number of indices
		
	protected:
		static auto getVertexBuffer() -> VertexBufferCPU;
		static auto getIndexBuffer() -> IndexBufferCPU;
	};

}
}
