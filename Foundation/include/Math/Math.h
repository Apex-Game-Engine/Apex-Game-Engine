#pragma once
#pragma message("Including Math.h")

#include "Core/Types.h"

namespace apex {
namespace math {
	struct Vector2;
	struct Vector3;
	struct Vector4;

	float32 sqrt(float32 val);
	float32 clamp(float32 val, float32 min, float32 max);
	float32 radians(float32 degrees);
	float32 degrees(float32 radians);

}
}

namespace apex {
namespace math {

#ifdef APEX_ENABLE_GLSL_MATH_TYPE_NAMES
	using vec2 = Vector2;
	using vec3 = Vector3;
	using vec4 = Vector4;
	using mat4 = Matrix4x4;
#endif

#ifdef APEX_RAYTRACING_DEFINITIONS

	// TODO: Move to separate file
	struct Ray
	{
		Point3D origin;
		Vector3 direction;

		float tMin, tMax;

		// parametric form of a ray
		Point3D at(float32 t) const;
	};

	struct BoundingVolume
	{
		virtual ~BoundingVolume() = default;
		virtual bool intersect(Ray const &ray) = 0;
	};

#endif

}
}

#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Math.inl"
#endif