#include "Math/Random.h"

#include "Core/Asserts.h"
#include "Math/Vector4.h"

namespace apex::math {

	namespace detail
	{
		static Random::mt_engine s_engine;
		static std::uniform_int_distribution<Random::mt_engine::result_type> s_distribution;
	}

	#pragma region SlowRandom implementation

	inline void Random::init()
	{
		if (std::random_device().entropy() > 0)
		{
			detail::s_engine.seed(std::random_device()());
		}
		else
		{
			axWarn("True random device NOT available");
			detail::s_engine.seed(static_cast<mt_engine::result_type>(std::time(nullptr)));
		}
	}

	inline int32 Random::randomInt32()
	{
		return static_cast<int32>(detail::s_distribution(detail::s_engine));
	}

	inline float32 Random::randomFloat32()
	{
		return static_cast<float32>(detail::s_distribution(detail::s_engine)) / static_cast<float32>(detail::s_distribution.max());
	}

	Vector2 Random::randomVector2()
	{
		return { randomFloat32(), randomFloat32() };
	}

	Vector3 Random::randomVector3()
	{
		return { randomFloat32(), randomFloat32(), randomFloat32() };
	}

	Vector4 Random::randomVector4()
	{
		return { randomFloat32(), randomFloat32(), randomFloat32(), randomFloat32() };
	}

	Vector3 Random::randomUnitSphereVector3()
	{
		while (true)
		{
			Vector3 p = randomVector3(-1, 1);
			if (p.length_squared() >= 1) continue;
			return p;
		}
	}

	Vector3 Random::randomUnitVector3()
	{
		return randomUnitSphereVector3().normalize_();
	}

	inline int32 Random::randomInt32(int32 min, int32 max)
	{
		axAssert(min < max);
		return min + randomInt32() % (max - min);
	}

	inline float32 Random::randomFloat32(float32 min, float32 max)
	{
		axAssert(min < max);
		return min + randomFloat32() * (max - min);
	}

	Vector2 Random::randomVector2(float32 min, float32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max) };
	}

	Vector3 Random::randomVector3(float32 min, float32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max) };
	}

	Vector4 Random::randomVector4(float32 min, float32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max) };
	}

#pragma endregion

	#pragma region FastRandom implementation

	namespace detail 
	{
		inline uint32 pcg_hash(uint32 input)
		{
		    uint32 state = input * 747796405u + 2891336453u;
		    uint32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		    return (word >> 22u) ^ word;
		}
	}

	inline int32 FastRandom::randomInt32(uint32 seed)
	{
		return static_cast<int32>(detail::pcg_hash(seed));
	}
	
	inline float32 FastRandom::randomFloat32(uint32 seed)
	{
		return static_cast<float32>(detail::pcg_hash(seed));
	}

	Vector2 FastRandom::randomVector2(uint32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed) };
	}

	Vector3 FastRandom::randomVector3(uint32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed), randomFloat32(seed) };
	}

	Vector4 FastRandom::randomVector4(uint32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed), randomFloat32(seed), randomFloat32(seed) };
	}

	Vector3 FastRandom::randomUnitSphereVector3(uint32 seed)
	{
		while (true)
		{
			Vector3 p = randomVector3(seed, -1, 1);
			if (p.length_squared() >= 1) continue;
			return p;
		}
	}

	Vector3 FastRandom::randomUnitVector3(uint32 seed)
	{
		return randomUnitSphereVector3(seed).normalize_();
	}

	inline int32 FastRandom::randomInt32(uint32 seed, int32 min, int32 max)
	{
		axAssert(min < max);
		return min + randomInt32(seed) % (max - min);
	}

	inline float32 FastRandom::randomFloat32(uint32 seed, float32 min, float32 max)
	{
		axAssert(min < max);
		return min + randomFloat32(seed) * (max - min);
	}

	Vector2 FastRandom::randomVector2(uint32 seed, float32 min, float32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}

	Vector3 FastRandom::randomVector3(uint32 seed, float32 min, float32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}

	Vector4 FastRandom::randomVector4(uint32 seed, float32 min, float32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}
}
