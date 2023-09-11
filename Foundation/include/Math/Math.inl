#pragma once
#pragma message("Including Math.inl")

#include <cmath>

#include "Core/Logging.h"

namespace apex {
namespace math {

	inline float32 sqrt(float32 val)
	{
		return ::sqrt(val);
	}

	inline float32 clamp(float32 val, float32 min, float32 max)
	{
		return (val < min)
				? min
				: (val > max)
					? max
					: val;
	}

	inline float32 radians(float32 degrees)
	{
		return degrees * constants::float32_PI / 180.f;
	}

	inline float32 degrees(float32 radians)
	{
		return radians * 180.f / constants::float32_PI;
	}

#ifdef APEX_RAYTRACING_DEFINITIONS

	inline Point3D Ray::at(float32 t) const
	{
		axAssert(tMin < t && t < tMax);
		return origin + t * direction;
	}

#endif

}
}
