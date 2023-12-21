#pragma once
#include "Containers/AxArray.h"

namespace apex {
namespace gfx {
	struct Mesh;

	class MeshManager
	{
	public:
		MeshManager();
		~MeshManager();

	private:
		AxArray<Mesh*> m_meshes;
	};


}
}
