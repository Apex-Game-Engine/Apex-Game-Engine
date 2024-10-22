#pragma once
#include "Graphics/Geometry/Mesh.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"

namespace apex {
namespace ecs {

	struct Transform
	{
		math::Vector3 position;
		math::Quat    rotation;
	};

	struct MeshRenderer
	{
		gfx::Mesh* pMesh;
		// technique/effect
	};

}
}
