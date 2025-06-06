﻿#pragma once
//#pragma message("Including Vector2.h")

#include "Core/Types.h"

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

		constexpr Vector2() = default;
		constexpr Vector2(f32 v0, f32 v1) : m_values{v0, v1} {}
		constexpr Vector2(f32 val): m_values{val, val} {}
		Vector2(f32 values[2]) : m_values{values[0], values[1]} {}

		#pragma region Overloaded operators

		f32 operator [](size_t index) const { return m_values[index]; }
		f32& operator [](size_t index) { return m_values[index]; }

		Vector2 operator-() const { return { -m_values[0],  -m_values[1] }; }

		Vector2& operator+=(Vector2 const &v); // element-wise vector addition
		Vector2& operator-=(Vector2 const &v); // element-wise vector subtraction

		Vector2& operator*=(Vector2 const &v); // element-wise vector multiplication
		Vector2& operator*=(f32 t);
		Vector2& operator/=(Vector2 const &v); // element-wise vector division
		Vector2& operator/=(f32 t);

		#pragma endregion

		[[nodiscard]] f32 length() const;
		[[nodiscard]] f32 lengthSquared() const;

		Vector2& normalize_();
		[[nodiscard]] Vector2 normalize() const;
		
		[[nodiscard]] bool isNearZero() const;
	};

	static_assert(sizeof(Vector2) == 2 * sizeof(float));

	// Vector2 utility functions

	Vector2 operator+(Vector2 const &u, Vector2 const &v); // element-wise addition of two vectors
	Vector2 operator-(Vector2 const &u, Vector2 const &v); // element-wise subtraction of two vectors
	Vector2 operator*(Vector2 const &u, Vector2 const &v); // element-wise multiplication of two vectors
	Vector2 operator*(f32        t, Vector2 const &v); // multiply each element of vector with scalar
	Vector2 operator*(Vector2 const &v, f32        t); // multiply each element of vector with scalar
	Vector2 operator/(Vector2 const &v, f32        t); // divide each element of vector by scalar
	Vector2 operator/(Vector2 const &u, Vector2 const &v); // element-wise division of two vectors

	bool operator==(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator!=(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator<(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator>(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator<=(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator>=(Vector2 const &u, Vector2 const &v); // element-wise comparison

	Vector2 normalize(Vector2 const &vec); // return a unit vector in the same direction as given vector
	Vector2& normalize_inplace(Vector2 &vec); // convert the given vector into a unit vector in same direction

	f32 dot(Vector2 const &u, Vector2 const &v);

	Vector2 min(Vector2 const& u, Vector2 const& v);
	Vector2 max(Vector2 const& u, Vector2 const& v);

}	
}

//#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Vector2.inl"
//#endif
