#pragma once
#include "Containers/AxArray.h"

namespace apex {
namespace gfx {

	class IndexBufferCPU 
	{
	public:
		constexpr IndexBufferCPU() = default;

		void create(uint32 const* indices, size_t size)
		{
			m_data.data = indices;
			m_data.count = size;
		}

		void create(AxArray<uint32> const& indices)
		{
			m_data.data = indices.data();
			m_data.count = indices.size();
		}

		size_t count() const { return m_data.count; }
		size_t sizeInBytes() const { return m_data.count * sizeof(uint32); }

		auto data() const { return m_data; }

	private:
		AxArrayRef<const uint32> m_data;
	};

}
}
