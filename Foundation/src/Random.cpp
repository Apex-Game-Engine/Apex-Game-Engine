#include "Math/Random.h"

#include "Core/Asserts.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include <random>

namespace apex::math {

	using mt_engine = std::mt19937;

	namespace detail
	{
		static mt_engine s_engine;
		static std::uniform_int_distribution<mt_engine::result_type> s_distribution;
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

	inline s32 Random::randomInt32()
	{
		return static_cast<s32>(detail::s_distribution(detail::s_engine));
	}

	inline f32 Random::randomFloat32()
	{
		return static_cast<f32>(detail::s_distribution(detail::s_engine)) / static_cast<f32>(detail::s_distribution.max());
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
			if (p.lengthSquared() >= 1) continue;
			return p;
		}
	}

	Vector3 Random::randomUnitVector3()
	{
		return randomUnitSphereVector3().normalize_();
	}

	inline s32 Random::randomInt32(s32 min, s32 max)
	{
		axAssert(min < max);
		return min + randomInt32() % (max - min);
	}

	inline f32 Random::randomFloat32(f32 min, f32 max)
	{
		axAssert(min < max);
		return min + randomFloat32() * (max - min);
	}

	Vector2 Random::randomVector2(f32 min, f32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max) };
	}

	Vector3 Random::randomVector3(f32 min, f32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max) };
	}

	Vector4 Random::randomVector4(f32 min, f32 max)
	{
		return { randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max), randomFloat32(min, max) };
	}

#pragma endregion

	#pragma region FastRandom implementation

	namespace detail 
	{
		inline u32 pcg_hash(u32 input)
		{
		    u32 state = input * 747796405u + 2891336453u;
		    u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		    return (word >> 22u) ^ word;
		}
	}

	inline s32 FastRandom::randomInt32(u32 seed)
	{
		return static_cast<s32>(detail::pcg_hash(seed));
	}
	
	inline f32 FastRandom::randomFloat32(u32 seed)
	{
		return static_cast<f32>(detail::pcg_hash(seed));
	}

	Vector2 FastRandom::randomVector2(u32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed) };
	}

	Vector3 FastRandom::randomVector3(u32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed), randomFloat32(seed) };
	}

	Vector4 FastRandom::randomVector4(u32 seed)
	{
		return { randomFloat32(seed), randomFloat32(seed), randomFloat32(seed), randomFloat32(seed) };
	}

	Vector3 FastRandom::randomUnitSphereVector3(u32 seed)
	{
		while (true)
		{
			Vector3 p = randomVector3(seed, -1, 1);
			if (p.lengthSquared() >= 1) continue;
			return p;
		}
	}

	Vector3 FastRandom::randomUnitVector3(u32 seed)
	{
		return randomUnitSphereVector3(seed).normalize_();
	}

	inline s32 FastRandom::randomInt32(u32 seed, s32 min, s32 max)
	{
		axAssert(min < max);
		return min + randomInt32(seed) % (max - min);
	}

	inline f32 FastRandom::randomFloat32(u32 seed, f32 min, f32 max)
	{
		axAssert(min < max);
		return min + randomFloat32(seed) * (max - min);
	}

	Vector2 FastRandom::randomVector2(u32 seed, f32 min, f32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}

	Vector3 FastRandom::randomVector3(u32 seed, f32 min, f32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}

	Vector4 FastRandom::randomVector4(u32 seed, f32 min, f32 max)
	{
		return { randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max), randomFloat32(seed, min, max) };
	}
}
