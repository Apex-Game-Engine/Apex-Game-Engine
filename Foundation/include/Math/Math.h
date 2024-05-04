#pragma once
//#pragma message("Including Math.h")

#include "Core/Types.h"

namespace apex {
namespace math {
	float32 sqrt(float32 val);
	float32 clamp(float32 val, float32 min, float32 max);
	float32 radians(float32 degrees);
	float32 degrees(float32 radians);

	template <typename T> requires std::is_integral_v<T>
	constexpr bool is_pow_2(T val)
	{
		return val != 0 && (val & (val - 1)) == 0;
	}

	template <typename T> requires std::is_arithmetic_v<T>
	T ceil(T val, T radix)
	{
		if constexpr (std::is_floating_point_v<T>)
			return std::ceil(val / radix);
		else // if constexpr (std::is_integral_v<T>)
			return (val + radix - 1) / radix;
	}

	template <typename T> requires std::is_integral_v<T>
	T round_to(T val, T radix)
	{
		//static_assert(math::is_pow_2(radix), "Radix must be a power of 2");
		return (val + radix - 1) & ~(radix - 1);
	}
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