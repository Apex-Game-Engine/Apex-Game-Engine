#pragma once
#include <cmath>

#include "Core/Logging.h"
#include "Core/Asserts.h"

namespace apex {
namespace math {

	namespace detail 
	{
		inline uint32 pcg_hash(uint32 input)
		{
		    uint32 state = input * 747796405u + 2891336453u;
		    uint32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		    return (word >> 22u) ^ word;
		}
	}

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

	inline float32 degrees_to_radians(float32 degrees)
	{
		return degrees * constants::float32_PI / 180.f;
	}

	inline float32 radians_to_degrees(float32 radians)
	{
		return radians * 180.f / constants::float32_PI;
	}
	
	#pragma region SlowRandom implementation

	inline void Random::init()
	{
		if (std::random_device().entropy() > 0)
		{
			s_engine.seed(std::random_device()());
		}
		else
		{
			axWarn("True random device NOT available");
			s_engine.seed(static_cast<mt_engine::result_type>(std::time(nullptr)));
		}
	}

	inline int32 Random::getInt32()
	{
		return static_cast<int32>(s_distribution(s_engine));
	}

	inline float32 Random::getFloat32()
	{
		return static_cast<float32>(s_distribution(s_engine)) / static_cast<float32>(s_distribution.max());
	}

	inline int32 Random::getInt32(int32 min, int32 max)
	{
		axAssert(min < max);
		return min + getInt32() % (max - min);
	}

	inline float32 Random::getFloat32(float32 min, float32 max)
	{
		axAssert(min < max);
		return min + getFloat32() * (max - min);
	}

	#pragma endregion

	#pragma region FastRandom implementation

	inline int32 FastRandom::getInt32(uint32 seed)
	{
		return static_cast<int32>(detail::pcg_hash(seed));
	}
	
	inline float32 FastRandom::getFloat32(uint32 seed)
	{
		return static_cast<float32>(detail::pcg_hash(seed));
	}

	inline int32 FastRandom::getInt32(uint32 seed, int32 min, int32 max)
	{
		axAssert(min < max);
		return min + getInt32(seed) % (max - min);
	}

	inline float32 FastRandom::getFloat32(uint32 seed, float32 min, float32 max)
	{
		axAssert(min < max);
		return min + getFloat32(seed) * (max - min);
	}

	#pragma endregion

	inline Point3D Ray::at(float32 t) const
	{
		axAssert(tMin < t && t < tMax);
		return origin + t * direction;
	}

}
}
