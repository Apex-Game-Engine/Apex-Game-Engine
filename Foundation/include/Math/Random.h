#pragma once

#include "Core/Types.h"
// #include "Vector3.h"
#include "Vector-Fwd.h"

namespace apex {
namespace math {

	struct Random
	{
		static void init();

		static s32 randomInt32();
		static f32 randomFloat32();
		static Vector2 randomVector2();
		static Vector3 randomVector3();
		static Vector4 randomVector4();
		static Vector3 randomUnitSphereVector3();
		static Vector3 randomUnitVector3();

		static s32 randomInt32(s32 min, s32 max);
		static f32 randomFloat32(f32 min, f32 max);
		static Vector2 randomVector2(f32 min, f32 max);
		static Vector3 randomVector3(f32 min, f32 max);
		static Vector4 randomVector4(f32 min, f32 max);
	};

	struct FastRandom
	{
		static s32 randomInt32(u32 seed);
		static f32 randomFloat32(u32 seed);
		static Vector2 randomVector2(u32 seed);
		static Vector3 randomVector3(u32 seed);
		static Vector4 randomVector4(u32 seed);
		static Vector3 randomUnitSphereVector3(u32 seed);
		static Vector3 randomUnitVector3(u32 seed);

		static s32 randomInt32(u32 seed, s32 min, s32 max);
		static f32 randomFloat32(u32 seed, f32 min, f32 max);
		static Vector2 randomVector2(u32 seed, f32 min, f32 max);
		static Vector3 randomVector3(u32 seed, f32 min, f32 max);
		static Vector4 randomVector4(u32 seed, f32 min, f32 max);
	};

}
}
