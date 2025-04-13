#pragma once
//#pragma message("Including Vector4.h")

#include "VectorDefines.h"
#include "Core/Types.h"

#include "Vector3.h"

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

		constexpr Vector4() = default;
		constexpr Vector4(f32 v0, f32 v1, f32 v2, f32 v3) : m_values{v0, v1, v2, v3} {}
		constexpr explicit Vector4(f32 val) : m_values{val, val, val, val} {}
		Vector4(f32 values[4]) : m_values{values[0], values[1], values[2], values[3]} {}

		// Ctors from Vector3
		Vector4(Vector3 const &_xyz, f32 _w) : m_values{_xyz[0], _xyz[1], _xyz[2], _w} {}

		// Ctors from Vector2
		Vector4(Vector2 const &_xy, f32         _z, f32         _w) : m_values{_xy[0], _xy[1], _z, _w} {}
		Vector4(f32         _x, Vector2 const &_yz, f32         _w) : m_values{_x, _yz[0], _yz[1], _w} {}
		Vector4(f32         _x, f32         _y, Vector2 const &_zw) : m_values{_x, _y, _zw[0], _zw[1]} {}
		Vector4(Vector2 const &_xy, Vector2 const &_zw) : m_values{_xy[0], _xy[1], _zw[0], _zw[1]} {}

		#pragma region Overloaded operators

		f32 operator [](size_t index) const { return m_values[index]; }
		f32& operator [](size_t index) { return m_values[index]; }

		Vector4 operator-() const { return { -m_values[0],  -m_values[1], -m_values[2], -m_values[3] }; }

		Vector4& operator+=(Vector4 const &v);
		Vector4& operator-=(Vector4 const &v);

		Vector4& operator*=(Vector4 const &v);
		Vector4& operator*=(f32 t);
		Vector4& operator/=(f32 t);

		#pragma endregion

		static constexpr Vector4 unitX() { return { 1.f, 0.f, 0.f, 0.f }; }
		static constexpr Vector4 unitY() { return { 0.f, 1.f, 0.f, 0.f }; }
		static constexpr Vector4 unitZ() { return { 0.f, 0.f, 1.f, 0.f }; }
		static constexpr Vector4 unitW() { return { 0.f, 0.f, 0.f, 1.f }; }

		static constexpr Vector4 unitX_w1() { return { 1.f, 0.f, 0.f, 1.f }; }
		static constexpr Vector4 unitY_w1() { return { 0.f, 1.f, 0.f, 1.f }; }
		static constexpr Vector4 unitZ_w1() { return { 0.f, 0.f, 1.f, 1.f }; }

		APEX_MATH_VECTOR3_SWIZZLES(x, y, z)
	};

	static_assert(sizeof(Vector4) == 4 * sizeof(float));

	// Vector4 utility functions

	Vector4 operator+(Vector4 const &u, Vector4 const &v); // element-wise addition of two vectors
	Vector4 operator-(Vector4 const &u, Vector4 const &v); // element-wise subtraction of two vectors
	Vector4 operator*(Vector4 const &u, Vector4 const &v); // element-wise multiplication of two vectors
	Vector4 operator*(f32 t, Vector4 const &v); // multiply each element of vector with scalar
	Vector4 operator*(Vector4 const &v, f32 t); // multiply each element of vector with scalar
	Vector4 operator/(Vector4 const &v, f32 t); // divide each element of vector by scalar

	bool operator==(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator!=(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator<(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator>(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator<=(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator>=(Vector4 const &u, Vector4 const &v); // element-wise comparison

	f32 dot(Vector4 const &u, Vector4 const &v); // dot product

	Vector4 min(Vector4 const& u, Vector4 const& v);
	Vector4 max(Vector4 const& u, Vector4 const& v);
}
}


//#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Vector4.inl"
//#endif
