#pragma once
//#pragma message("Including Math.inl")

#include <cmath>

#include "Core/Logging.h"

namespace apex {
namespace math {

	inline f32 sqrt(f32 val)
	{
		return ::sqrt(val);
	}

	inline f32 clamp(f32 val, f32 min, f32 max)
	{
		return (val < min)
				? min
				: (val > max)
					? max
					: val;
	}

	inline f32 radians(f32 degrees)
	{
		return degrees * Constants::f32_PI / 180.f;
	}

	inline f32 degrees(f32 radians)
	{
		return radians * 180.f / Constants::f32_PI;
	}

	inline f32 sign(f32 val)
	{
		return static_cast<f32>((val > 0.f) - (val < 0.f));
		//return std::signbit(val) ? -1.f : 1.f;
	}

#ifdef APEX_RAYTRACING_DEFINITIONS

	inline Point3D Ray::at(f32 t) const
	{
		axAssert(tMin < t && t < tMax);
		return origin + t * direction;
	}

#endif

}
}
