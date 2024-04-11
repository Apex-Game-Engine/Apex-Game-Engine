#pragma once
#include "Containers/AxArray.h"

namespace apex {
namespace gfx {
	struct StaticMesh;

	class MeshManager
	{
	public:
		MeshManager();
		~MeshManager();

	private:
		AxArray<StaticMesh*> m_meshes;
	};


}
}
