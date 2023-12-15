#pragma once
//#pragma message("Including Vector3.inl")

#include <immintrin.h>

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
		return apex::math::dot(*this, *this);
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
		__m128 mU = _mm_loadu_ps(u.m_values);
		__m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_add_ps(mU, mV);

		float res[4];
		_mm_storeu_ps(res, mRes);
		return res;

		return { u.x + v.x, u.y + v.y, u.z + v.z };
	}

	inline Vector3 operator-(Vector3 const& u, Vector3 const& v)
	{
		__m128 mU = _mm_loadu_ps(u.m_values);
		__m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_sub_ps(mU, mV);

		float res[4];
		_mm_storeu_ps(res, mRes);
		return res;

		return { u.x - v.x, u.y - v.y, u.z - v.z };
	}

	inline Vector3 operator*(Vector3 const& u, Vector3 const& v)
	{
		__m128 mU = _mm_loadu_ps(u.m_values);
        __m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_mul_ps(mU, mV);

		float res[4];
		_mm_storeu_ps(res, mRes);
		return res;

		return { u.x * v.x, u.y * v.y, u.z * v.z };
	}

	inline Vector3 operator*(float32 t, Vector3 const& v)
	{
		__m128 mT = _mm_set1_ps(t);
        __m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_mul_ps(mT, mV);

		float res[4];
		_mm_storeu_ps(res, mRes);
		return res;

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
		__m128 mU = _mm_loadu_ps(u.m_values);
		__m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_dp_ps(mU, mV, 0x71 /* mask */);

		// mask: 0111 0001 (use the lower 3 positions for dot product & save result in only the first position)
		// the high bits define the condition for dot product - only positions with 1 in the high bits are used
		// the low bits define the broadcast - broadcast the result to all positions with 1 in the low bits

		float res = _mm_cvtss_f32(mRes);
		return res;

		return u.x * v.x
			 + u.y * v.y
			 + u.z * v.z;
	}

	inline Vector3 cross(Vector3 const& u, Vector3 const& v)
	{
		__m128 mU = _mm_loadu_ps(u.m_values);
		__m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_sub_ps(
			_mm_mul_ps(_mm_permute_ps(mU, _MM_SHUFFLE(3, 0, 2, 1)), _mm_permute_ps(mV, _MM_SHUFFLE(3, 1, 0, 2))),
			_mm_mul_ps(_mm_permute_ps(mU, _MM_SHUFFLE(3, 1, 0, 2)), _mm_permute_ps(mV, _MM_SHUFFLE(3, 0, 2, 1)))
		);

		float res[4];
		_mm_storeu_ps(res, mRes);
		return res;

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
