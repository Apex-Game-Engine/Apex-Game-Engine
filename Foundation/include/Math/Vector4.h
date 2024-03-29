﻿#pragma once
//#pragma message("Including Vector4.h")

#include "VectorDefines.h"
#include "Core/Types.h"

#ifndef APEX_MATH_SKIP_INLINE_IMPL
#	define APEX_MATH_SKIP_INLINE_IMPL
#	include "Vector3.h"
#	undef APEX_MATH_SKIP_INLINE_IMPL
#else
#	include "Vector3.h"
#endif

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

		constexpr Vector4() = default;
		constexpr Vector4(float32 v0, float32 v1, float32 v2, float32 v3) : m_values{v0, v1, v2, v3} {}
		constexpr explicit Vector4(float32 val) : m_values{val, val, val, val} {}
		Vector4(float32 values[4]) : m_values{values[0], values[1], values[2], values[3]} {}

		// Ctors from Vector3
		Vector4(Vector3 const &_xyz, float32 _w) : m_values{_xyz[0], _xyz[1], _xyz[2], _w} {}

		// Ctors from Vector2
		Vector4(Vector2 const &_xy, float32         _z, float32         _w) : m_values{_xy[0], _xy[1], _z, _w} {}
		Vector4(float32         _x, Vector2 const &_yz, float32         _w) : m_values{_x, _yz[0], _yz[1], _w} {}
		Vector4(float32         _x, float32         _y, Vector2 const &_zw) : m_values{_x, _y, _zw[0], _zw[1]} {}
		Vector4(Vector2 const &_xy, Vector2 const &_zw) : m_values{_xy[0], _xy[1], _zw[0], _zw[1]} {}

		#pragma region Overloaded operators

		float32 operator [](size_t index) const { return m_values[index]; }
		float32& operator [](size_t index) { return m_values[index]; }

		Vector4 operator-() const { return { -m_values[0],  -m_values[1], -m_values[2], -m_values[3] }; }

		Vector4& operator+=(Vector4 const &v);
		Vector4& operator-=(Vector4 const &v);

		Vector4& operator*=(Vector4 const &v);
		Vector4& operator*=(float32 t);
		Vector4& operator/=(float32 t);

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
	Vector4 operator*(float32 t, Vector4 const &v); // multiply each element of vector with scalar
	Vector4 operator*(Vector4 const &v, float32 t); // multiply each element of vector with scalar
	Vector4 operator/(Vector4 const &v, float32 t); // divide each element of vector by scalar

	bool operator==(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator!=(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator<(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator>(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator<=(Vector4 const &u, Vector4 const &v); // element-wise comparison
	bool operator>=(Vector4 const &u, Vector4 const &v); // element-wise comparison

	float32 dot(Vector4 const &u, Vector4 const &v); // dot product
}
}


#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Vector4.inl"
#endif
