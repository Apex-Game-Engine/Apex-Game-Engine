#pragma once
#pragma message("Including Vector3.inl")

#include "Vector3.h"
#include "Math.h"

namespace apex {
namespace math {

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
		return floatCompareNearZero(m_values[0])
			&& floatCompareNearZero(m_values[1])
			&& floatCompareNearZero(m_values[2]);
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
		return floatCompareApproximatelyEqual(u.x, v.x)
			&& floatCompareApproximatelyEqual(u.y, v.y)
			&& floatCompareApproximatelyEqual(u.z, v.z);
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

}
}
