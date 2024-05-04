#pragma once

#include "Core/Types.h"
// #include "Vector3.h"
#include "Vector-Fwd.h"

namespace apex {
namespace math {

	struct Random
	{
		static void init();

		static int32 randomInt32();
		static float32 randomFloat32();
		static Vector2 randomVector2();
		static Vector3 randomVector3();
		static Vector4 randomVector4();
		static Vector3 randomUnitSphereVector3();
		static Vector3 randomUnitVector3();

		static int32 randomInt32(int32 min, int32 max);
		static float32 randomFloat32(float32 min, float32 max);
		static Vector2 randomVector2(float32 min, float32 max);
		static Vector3 randomVector3(float32 min, float32 max);
		static Vector4 randomVector4(float32 min, float32 max);
	};

	struct FastRandom
	{
		static int32 randomInt32(uint32 seed);
		static float32 randomFloat32(uint32 seed);
		static Vector2 randomVector2(uint32 seed);
		static Vector3 randomVector3(uint32 seed);
		static Vector4 randomVector4(uint32 seed);
		static Vector3 randomUnitSphereVector3(uint32 seed);
		static Vector3 randomUnitVector3(uint32 seed);

		static int32 randomInt32(uint32 seed, int32 min, int32 max);
		static float32 randomFloat32(uint32 seed, float32 min, float32 max);
		static Vector2 randomVector2(uint32 seed, float32 min, float32 max);
		static Vector3 randomVector3(uint32 seed, float32 min, float32 max);
		static Vector4 randomVector4(uint32 seed, float32 min, float32 max);
	};

}
}
