#pragma once
#include "Core/Types.h"
#include "Vector3.h"

namespace apex {
namespace math {

	struct Vector4
	{
		union
		{
			struct { float32 x, y, z, w; };
			struct { float32 r, g, b, a; };
			struct { float32 s, t, u, v; };

			float32 m_values[4] = { 0.f, 0.f, 0.f, 0.f };
		};

		Vector4() = default;
		Vector4(float32 v0, float32 v1, float32 v2, float32 v3) : m_values{v0, v1, v2, v3} {}
		Vector4(float32 values[4]) : m_values{values[0], values[1], values[2], values[3]} {}
		Vector4(float32 val) : m_values{val, val, val, val} {}

		// Ctors from Vector3
		Vector4(Vector3 const &_xyz, float32 _w) : m_values{_xyz[0], _xyz[1], _xyz[2], _w} {}

		// Ctors from Vector2
		Vector4(Vector2 const &_xy, float32         _z, float32         _w) : m_values{_xy[0], _xy[1], _z, _w} {}
		Vector4(float32         _x, Vector2 const &_yz, float32         _w) : m_values{_x, _yz[0], _yz[1], _w} {}
		Vector4(float32         _x, float32         _y, Vector2 const &_zw) : m_values{_x, _y, _zw[0], _zw[1]} {}
		Vector4(Vector2 const &_xy, Vector2 const &_zw) : m_values{_xy[0], _xy[1], _zw[0], _zw[1]} {}

		float32 operator [](size_t index) const { return m_values[index]; }
		float32& operator [](size_t index) { return m_values[index]; }


	};
}
}
