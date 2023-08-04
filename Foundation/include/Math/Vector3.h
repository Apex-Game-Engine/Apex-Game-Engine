#pragma once
#include "Core/Types.h"

namespace apex {
namespace math {

	struct Vector2
	{
		union
		{
			struct { float32 x, y; };
			struct { float32 r, g; };
			struct { float32 s, t; };

			float32 m_values[2] = { 0.f, 0.f };
		};

		Vector2() = default;
		Vector2(float32 v0, float32 v1) : m_values{v0, v1} {}
		Vector2(float32 values[2]) : m_values{values[0], values[1]} {}
		Vector2(float32 val): m_values{val, val} {}

		#pragma region Overloaded operators

		float32 operator [](size_t index) const { return m_values[index]; }
		float32& operator [](size_t index) { return m_values[index]; }

		Vector2 operator-() const { return { -m_values[0],  -m_values[1] }; }

		Vector2& operator+=(Vector2 const &v); // element-wise vector addition
		Vector2& operator-=(Vector2 const &v); // element-wise vector subtraction

		Vector2& operator*=(Vector2 const &v); // element-wise vector multiplication
		Vector2& operator*=(float32 t);
		Vector2& operator/=(float32 t);

		#pragma endregion

		[[nodiscard]] float32 length() const;
		[[nodiscard]] float32 length_squared() const;

		Vector2& normalize_();
		[[nodiscard]] Vector2 normalize() const;
	};

	Vector2 operator+(Vector2 const &u, Vector2 const &v); // element-wise addition of two vectors
	Vector2 operator-(Vector2 const &u, Vector2 const &v); // element-wise subtraction of two vectors
	Vector2 operator*(Vector2 const &u, Vector2 const &v); // element-wise multiplication of two vectors
	Vector2 operator*(float32        t, Vector2 const &v); // multiply each element of vector with scalar
	Vector2 operator*(Vector2 const &v, float32        t); // multiply each element of vector with scalar
	Vector2 operator/(Vector2 const &v, float32        t); // divide each element of vector by scalar

	bool operator==(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator!=(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator<(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator>(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator<=(Vector2 const &u, Vector2 const &v); // element-wise comparison
	bool operator>=(Vector2 const &u, Vector2 const &v); // element-wise comparison

	Vector2 normalize(Vector2 const &vec); // return a unit vector in the same direction as given vector
	Vector2& normalize_inplace(Vector2 &vec); // convert the given vector into a unit vector in same direction

	float32 dot(Vector2 const &u, Vector2 const &v);

	// Vector3 utility functions

	struct Vector3
	{
		union
		{
			struct { float32 x, y, z; };
			struct { float32 r, g, b; };
			struct { float32 s, t, u; };

			float32 m_values[3] = { 0.f, 0.f, 0.f };
		};

		Vector3() = default;
		Vector3(float32 v0, float32 v1, float32 v2) : m_values{v0, v1, v2} {}
		Vector3(float32 values[3]) : m_values{values[0], values[1], values[2]} {}
		Vector3(float32 val) : m_values{val, val, val} {}

		// Ctors from Vector2
		Vector3(Vector2 const &_xy, float32 _z) : m_values{ _xy[0], _xy[1], _z} {}
		Vector3(float32 _x, Vector2 const &_yz) : m_values{ _x, _yz[0], _yz[1]} {}

		[[nodiscard]] Vector2 xy() const { return { x, y }; }
		[[nodiscard]] Vector2 yx() const { return { y, x }; }
		[[nodiscard]] Vector2 yz() const { return { y, z }; }
		[[nodiscard]] Vector2 zy() const { return { z, y }; }
		[[nodiscard]] Vector2 zx() const { return { z, x }; }
		[[nodiscard]] Vector2 xz() const { return { x, z }; }

		#pragma region Overloaded operators

		float32 operator [](size_t index) const { return m_values[index]; }
		float32& operator [](size_t index) { return m_values[index]; }

		Vector3 operator-() const { return { -m_values[0],  -m_values[1], -m_values[2] }; }

		Vector3& operator+=(Vector3 const &v);
		Vector3& operator-=(Vector3 const &v);

		Vector3& operator*=(Vector3 const &v);
		Vector3& operator*=(float32 t);
		Vector3& operator/=(float32 t);

		#pragma endregion

		[[nodiscard]] float32 length() const;
		[[nodiscard]] float32 length_squared() const;

		Vector3& normalize_();
		[[nodiscard]] Vector3 normalize() const;

		[[nodiscard]] bool is_near_zero() const;

		static Vector3 random();
		static Vector3 random(float32 min, float32 max);
		static Vector3 random_in_unit_sphere();
		static Vector3 random_unit_vector();
		static bool check_near_zero(Vector3 const &v);
	};

	// Vector3 utility functions

	Vector3 operator+(Vector3 const &u, Vector3 const &v); // element-wise addition of two vectors
	Vector3 operator-(Vector3 const &u, Vector3 const &v); // element-wise subtraction of two vectors
	Vector3 operator*(Vector3 const &u, Vector3 const &v); // element-wise multiplication of two vectors
	Vector3 operator*(float32 t, Vector3 const &v); // multiply each element of vector with scalar
	Vector3 operator*(Vector3 const &v, float32 t); // multiply each element of vector with scalar
	Vector3 operator/(Vector3 const &v, float32 t); // divide each element of vector by scalar

	bool operator==(Vector3 const &u, Vector3 const &v); // element-wise comparison
	bool operator!=(Vector3 const &u, Vector3 const &v); // element-wise comparison
	bool operator<(Vector3 const &u, Vector3 const &v); // element-wise comparison
	bool operator>(Vector3 const &u, Vector3 const &v); // element-wise comparison
	bool operator<=(Vector3 const &u, Vector3 const &v); // element-wise comparison
	bool operator>=(Vector3 const &u, Vector3 const &v); // element-wise comparison

	Vector3 normalize(Vector3 const &vec); // return a unit vector in the same direction as given vector
	Vector3& normalize_inplace(Vector3 &vec); // convert the given vector into a unit vector in same direction

	float32 dot(Vector3 const &u, Vector3 const &v);
	Vector3 cross(Vector3 const &u, Vector3 const &v);

	Vector3 reflect(Vector3 const &u, Vector3 const &n);
	Vector3 refract(Vector3 const &u, Vector3 const &n, float32 refraction_ratio);


	// Additional typedefs
	using Point2D = Vector2;
	using Point3D = Vector3;

}
}


#ifndef SKIP_INLINE_MATH
#include "Vector3.inl"
#endif
