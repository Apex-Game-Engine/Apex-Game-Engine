#pragma once
#include "Foundation/axTypes.h"

namespace apex {
namespace math {

	struct Vector2
	{
		union
		{
			struct { f32 x, y; };
			struct { f32 r, g; };
			struct { f32 s, t; };

			f32 m_values[2] = { 0.f, 0.f };
		};

		Vector2() = default;
		Vector2(f32, f32);
		Vector2(f32[2]);
		Vector2(f32);

		f32& operator [](size_t index);
		const f32& operator [](size_t index) const;
	};

	struct Vector3
	{
		union
		{
			struct { f32 x, y, z; };
			struct { f32 r, g, b; };
			struct { f32 s, t, u; };

			f32 m_values[3] = { 0.f, 0.f, 0.f };
		};

		Vector3() = default;
		Vector3(f32, f32, f32);
		Vector3(f32[3]);
		Vector3(f32);

		// Ctors from Vector2
		Vector3(Vector2 const&,            f32);
		Vector3(           f32, Vector2 const&);

		f32& operator [](size_t index);
		const f32& operator [](size_t index) const;

	};

}
}
