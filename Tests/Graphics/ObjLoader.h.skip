#pragma once

#include <tinyobj_loader_c.h>

#include "Containers/AxArray.h"


namespace apex {

	class ObjLoader
	{
	public:
		ObjLoader() = default;

		ObjLoader(const char* filename) { LoadFile(filename); }

		void LoadFile(const char* filename);

		AxArrayRef<float> GetVertexBufferData();
		AxArrayRef<u32> GetIndexBufferData();

		AxArrayRef<const float> GetVertexBufferData() const;
		AxArrayRef<const u32> GetIndexBufferData() const;

	private:
		AxArray<float> m_vertices;
		AxArray<u32> m_indices;
	};

}

