#pragma once
#include "Math.h"

namespace apex {
namespace math {

	/**************************************************************
	 * Vector2
	 *************************************************************/

	#pragma region Vector2 member functions

	inline Vector2& Vector2::operator+=(Vector2 const& v)
	{
		m_values[0] += v.m_values[0];
		m_values[1] += v.m_values[1];
		return *this;
	}

	inline Vector2& Vector2::operator-=(Vector2 const& v)
	{
		m_values[0] -= v.m_values[0];
		m_values[1] -= v.m_values[1];
		return *this;
	}

	inline Vector2& Vector2::operator*=(Vector2 const& v)
	{
		m_values[0] *= v.m_values[0];
		m_values[1] *= v.m_values[1];
		return *this;
	}

	inline Vector2& Vector2::operator*=(float32 t)
	{
		m_values[0] *= t;
		m_values[1] *= t;
		return *this;
	}

	inline Vector2& Vector2::operator/=(float32 t)
	{
		return (*this) *= 1/t;
	}

	inline float32 Vector2::length() const
	{
		return apex::math::sqrt(length_squared());
	}

	inline float32 Vector2::length_squared() const
	{
		return x*x + y*y;
	}

	inline Vector2 Vector2::normalize() const
	{
		return apex::math::normalize(*this);
	}

	inline Vector2& Vector2::normalize_()
	{
		return apex::math::normalize_inplace(*this);
	}

	#pragma endregion

	#pragma region Vector2 utility functions

	inline Vector2 operator+(Vector2 const& u, Vector2 const& v)
	{
		return { u.x + v.x, u.y + v.y };
	}

	inline Vector2 operator-(Vector2 const& u, Vector2 const& v)
	{
		return { u.x - v.x, u.y - v.y };
	}

	inline Vector2 operator*(Vector2 const& u, Vector2 const& v)
	{
		return { u.x * v.x, u.y * v.y };
	}

	inline Vector2 operator*(float32 t, Vector2 const& v)
	{
		return { t * v.x, t * v.y };
	}

	inline Vector2 operator*(Vector2 const& v, float32 t)
	{
		return t * v;
	}

	inline Vector2 operator/(Vector2 const& v, float32 t)
	{
		return (1/t) * v;
	}

	inline bool operator==(Vector2 const& u, Vector2 const& v)
	{
		return floatCompareAlmostEqual(u.x, v.x)
			&& floatCompareAlmostEqual(u.y, v.y);
	}

	inline bool operator!=(Vector2 const& u, Vector2 const& v)
	{
		return !(u == v);
	}

	inline bool operator<(Vector2 const& u, Vector2 const& v)
	{
		return u.x < v.x
			&& u.y < v.y;
	}

	inline bool operator>(Vector2 const& u, Vector2 const& v)
	{
		return v < u;
	}

	inline bool operator<=(Vector2 const& u, Vector2 const& v)
	{
		return !(v < u);
	}

	inline bool operator>=(Vector2 const& u, Vector2 const& v)
	{
		return !(u < v);
	}

	inline Vector2 normalize(Vector2 const& vec)
	{
		return vec / vec.length();
	}

	inline Vector2& normalize_inplace(Vector2& vec)
	{
		return vec /= vec.length();
	}

	inline float32 dot(Vector2 const& u, Vector2 const& v)
	{
		return u.x * v.x
			 + u.y * v.y;
	}

#pragma endregion

	/**************************************************************
	 * Vector3
	 *************************************************************/

	#pragma region Vector3 member functions

	inline Vector3& Vector3::operator+=(Vector3 const& v)
	{
		m_values[0] += v.m_values[0];
		m_values[1] += v.m_values[1];
		m_values[2] += v.m_values[2];
		return *this;
	}

	inline Vector3& Vector3::operator-=(Vector3 const& v)
	{
		m_values[0] -= v.m_values[0];
		m_values[1] -= v.m_values[1];
		m_values[2] -= v.m_values[2];
		return *this;
	}

	inline Vector3& Vector3::operator*=(Vector3 const &v)
	{
		m_values[0] *= v.m_values[0];
		m_values[1] *= v.m_values[1];
		m_values[2] *= v.m_values[0];
		return *this;
	}

	inline Vector3& Vector3::operator*=(const float32 t)
	{
		m_values[0] *= t;
		m_values[1] *= t;
		m_values[2] *= t;
		return *this;
	}

	inline Vector3& Vector3::operator/=(const float32 t)
	{
		return (*this) *= 1/t;
	}

	inline float32 Vector3::length() const
	{
		return apex::math::sqrt(length_squared());
	}

	inline float32 Vector3::length_squared() const
	{
		return x*x + y*y + z*z;
	}

	inline Vector3 Vector3::normalize() const
	{
		return apex::math::normalize(*this);
	}

	inline Vector3& Vector3::normalize_()
	{
		return apex::math::normalize_inplace(*this);
	}

	inline bool Vector3::is_near_zero() const
	{
		return check_near_zero(*this);
	}

	inline Vector3 Vector3::random()
	{
		return { Random::getFloat32(), Random::getFloat32(), Random::getFloat32() };
	}

	inline Vector3 Vector3::random(float32 min, float32 max)
	{
		return { Random::getFloat32(min, max), Random::getFloat32(min, max), Random::getFloat32(min, max) };
	}

	inline Vector3 Vector3::random_in_unit_sphere()
	{
		while (true)
		{
			Vector3 p = Vector3::random(-1, 1);
			if (p.length_squared() >= 1) continue;
			return p;
		}
	}

	inline Vector3 Vector3::random_unit_vector()
	{
		return random_in_unit_sphere().normalize_();
	}

	inline bool Vector3::check_near_zero(Vector3 const& v)
	{
		return (fabsf(v.m_values[0]) < constants::float32_EPSILON)
			&& (fabsf(v.m_values[1]) < constants::float32_EPSILON)
			&& (fabsf(v.m_values[2]) < constants::float32_EPSILON);
	}

#pragma endregion

	#pragma region Vector3 utility functions

	inline Vector3 operator+(Vector3 const& u, Vector3 const& v)
	{
		return { u.x + v.x, u.y + v.y, u.z + v.z };
	}

	inline Vector3 operator-(Vector3 const& u, Vector3 const& v)
	{
		return { u.x - v.x, u.y - v.y, u.z - v.z };
	}

	inline Vector3 operator*(Vector3 const& u, Vector3 const& v)
	{
		return { u.x * v.x, u.y * v.y, u.z * v.z };
	}

	inline Vector3 operator*(float32 t, Vector3 const& v)
	{
		return { t * v.x, t * v.y, t * v.z };
	}

	inline Vector3 operator*(Vector3 const& v, float32 t)
	{
		return t * v;
	}

	inline Vector3 operator/(Vector3 const& v, float32 t)
	{
		return (1/t) * v;
	}

	inline bool operator==(Vector3 const& u, Vector3 const& v)
	{
		return u.x == v.x
			&& u.y == v.y
			&& u.z == v.z;
	}

	inline bool operator!=(Vector3 const& u, Vector3 const& v)
	{
		return !(u == v);
	}

	inline bool operator<(Vector3 const& u, Vector3 const& v)
	{
		return u.x < v.x
			&& u.y < v.y
			&& u.z < v.z;
	}

	inline bool operator>(Vector3 const& u, Vector3 const& v)
	{
		return v < u;
	}

	inline bool operator<=(Vector3 const& u, Vector3 const& v)
	{
		return !(v < u);
	}

	inline bool operator>=(Vector3 const& u, Vector3 const& v)
	{
		return !(u < v);
	}

	inline Vector3 normalize(Vector3 const& vec)
	{
		return vec / vec.length();
	}

	inline Vector3& normalize_inplace(Vector3& vec)
	{
		return vec /= vec.length();
	}

	inline float32 dot(Vector3 const& u, Vector3 const& v)
	{
		return u.x * v.x
			 + u.y * v.y
			 + u.z * v.z;
	}

	inline Vector3 cross(Vector3 const& u, Vector3 const& v)
	{
		return {
			u[1] * v[2] - u[2] * v[1],
			u[2] * v[0] - u[0] * v[2],
			u[0] * v[1] - u[1] * v[0],
		};
	}

	inline Vector3 reflect(Vector3 const& u, Vector3 const& n)
	{
		return u - 2 * dot(u, n) * n;
	}

	inline Vector3 refract(Vector3 const& u, Vector3 const& n, float32 refraction_ratio)
	{
		/*
		 *     i * sin(theta) = i' * sin(theta')
		 *	   sin(theta') = sin(theta) * (i / i')
		 *
		 *	   R' = R'_par + R'_perp
		 *	   R'_perp = (i / i') * (R + cos(theta) * n)
		 *	           = (i / i') * (R + (-R . n)) * n)
		 *	   R'_par = -sqrt(1 - |R'_perp|^2) * n
		 */

		float32 cos_theta = fmin(dot(-u, n), 1.f);
		Vector3 ray_out_perp = refraction_ratio * (u + cos_theta * n);
		Vector3 ray_out_par = -math::sqrt(fabs(1.f - ray_out_perp.length_squared())) * n;

		return ray_out_perp + ray_out_par;
	}

#pragma endregion

	/**************************************************************
	 * Vector4
	 *************************************************************/


}
}
