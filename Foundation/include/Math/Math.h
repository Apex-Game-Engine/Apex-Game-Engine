#pragma once

#include <random>

#include "Core/Types.h"

namespace apex {
namespace math {

	float32 sqrt(float32 val);
	float32 clamp(float32 val, float32 min, float32 max);
	float32 degrees_to_radians(float32 degrees);
	float32 radians_to_degrees(float32 radians);

	struct Random
	{
		using mt_engine = std::mt19937;

		static void init();

		static int32 getInt32();
		static float32 getFloat32();

		static int32 getInt32(int32 min, int32 max);
		static float32 getFloat32(float32 min, float32 max);

		inline static mt_engine s_engine;
		inline static std::uniform_int_distribution<mt_engine::result_type> s_distribution;
	};

	struct FastRandom
	{
		static int32 getInt32(uint32 seed);
		static float32 getFloat32(uint32 seed);

		static int32 getInt32(uint32 seed, int32 min, int32 max);
		static float32 getFloat32(uint32 seed, float32 min, float32 max);
	};

}
}

#include "Vector3.h"
#include "Vector4.h"

namespace apex {
namespace math {

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

}
}

#ifndef SKIP_INLINE_MATH
#include "Math.inl"
#endif