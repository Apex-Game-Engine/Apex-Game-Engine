#pragma once
#include "Vertex.h"
#include "Containers/AxArray.h"

namespace apex {
namespace gfx {

	class VertexBufferCPU 
	{
	public:
		constexpr VertexBufferCPU() = default;

		template <typename VertexType>
		void create(VertexType const* vertex_data, size_t size)
		{
			m_vertexInfo = VertexType::template getVertexInfo();
			m_vertexCount = size;
			m_data.data = reinterpret_cast<float const*>(vertex_data);
			m_data.count = vertex_data[0].size() * size;
		}

		template <typename VertexType>
		void create(AxArray<VertexType> const& vertex_data)
		{
			m_vertexInfo = VertexType::template getVertexInfo();
			m_vertexCount = vertex_data.size();
			m_data.data = reinterpret_cast<float const*>(vertex_data.data());
			m_data.count = vertex_data[0].size() * vertex_data.size();
		}

		size_t size() const { return m_data.count; }
		size_t sizeInBytes() const { return m_data.count * sizeof(float); }
		size_t count() const { return m_vertexCount; }

		size_t vertexSize() const { return m_vertexInfo.stride; }
		auto vertexInfo() const { return m_vertexInfo; }

		auto data() const { return m_data; }

	private:
		AxArrayRef<const float> m_data;
		VertexInfo m_vertexInfo;
		size_t m_vertexCount;
	};

}
}
