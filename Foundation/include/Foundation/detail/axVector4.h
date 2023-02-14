#pragma once
#include "Foundation/axTypes.h"
#include "axVector3.h"

namespace apex {
namespace math {

	struct Vector4
	{
		union
		{
			struct { f32 x, y, z, w; };
			struct { f32 r, g, b, a; };
			struct { f32 s, t, u, v; };

			f32 m_values[4] = { 0.f, 0.f, 0.f, 0.f };
		};

		Vector4() = default;
		Vector4(f32, f32, f32, f32);
		Vector4(f32[4]);
		Vector4(f32);

		// Ctors from Vector3
		Vector4(Vector3 const&,            f32);
		Vector4(           f32, Vector3 const&);

		// Ctors from Vector2
		Vector4(Vector2 const&,            f32,            f32);
		Vector4(           f32, Vector2 const&,            f32);
		Vector4(           f32,            f32, Vector2 const&);
		Vector4(Vector2 const&, Vector2 const&);

		f32& operator [](size_t index);
		const f32& operator [](size_t index) const;
	};
}
}
