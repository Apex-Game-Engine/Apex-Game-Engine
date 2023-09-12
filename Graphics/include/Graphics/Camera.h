#pragma once
#include "Math/Matrix4x4.h"

namespace apex {
namespace gfx {
	
	struct Camera
	{
		math::Matrix4x4 model;
		math::Matrix4x4 view;
		math::Matrix4x4 projection;
	};

}
}
