#pragma once
//#pragma message("Including Vector4.inl")

#include "Vector4.h"
#include "Core/Utility.h"

namespace apex {
namespace math {

	
	/**************************************************************
	 * Vector4
	 *************************************************************/

	#pragma region Vector4 member functions

	inline Vector4& Vector4::operator+=(Vector4 const& v)
	{
		m_values[0] += v.m_values[0];
		m_values[1] += v.m_values[1];
		m_values[2] += v.m_values[2];
		m_values[3] += v.m_values[3];
		return *this;
	}

	inline Vector4& Vector4::operator-=(Vector4 const& v)
	{
		m_values[0] -= v.m_values[0];
		m_values[1] -= v.m_values[1];
		m_values[2] -= v.m_values[2];
		m_values[3] -= v.m_values[3];
		return *this;
	}

	inline Vector4& Vector4::operator*=(Vector4 const &v)
	{
		m_values[0] *= v.m_values[0];
		m_values[1] *= v.m_values[1];
		m_values[2] *= v.m_values[0];
		m_values[3] *= v.m_values[3];
		return *this;
	}

	inline Vector4& Vector4::operator*=(const f32 t)
	{
		m_values[0] *= t;
		m_values[1] *= t;
		m_values[2] *= t;
		m_values[3] *= t;
		return *this;
	}

	inline Vector4& Vector4::operator/=(const f32 t)
	{
		return (*this) *= 1/t;
	}

#pragma endregion

	#pragma region Vector4 utility functions

	inline Vector4 operator+(Vector4 const& u, Vector4 const& v)
	{
		return { u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w };
	}

	inline Vector4 operator-(Vector4 const& u, Vector4 const& v)
	{
		return { u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w };
	}

	inline Vector4 operator*(Vector4 const& u, Vector4 const& v)
	{
		return { u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w };
	}

	inline Vector4 operator*(f32 t, Vector4 const& v)
	{
		return { t * v.x, t * v.y, t * v.z, t * v.w };
	}

	inline Vector4 operator*(Vector4 const& v, f32 t)
	{
		return t * v;
	}

	inline Vector4 operator/(Vector4 const& v, f32 t)
	{
		return (1/t) * v;
	}

	inline bool operator==(Vector4 const& u, Vector4 const& v)
	{
		return floatCompareApproximatelyEqual(u.x, v.x)
			&& floatCompareApproximatelyEqual(u.y, v.y)
			&& floatCompareApproximatelyEqual(u.z, v.z)
			&& floatCompareApproximatelyEqual(u.w, v.w);
	}

	inline bool operator!=(Vector4 const& u, Vector4 const& v)
	{
		return !(u == v);
	}

	inline bool operator<(Vector4 const& u, Vector4 const& v)
	{
		return u.x < v.x
			&& u.y < v.y
			&& u.z < v.z;
	}

	inline bool operator>(Vector4 const& u, Vector4 const& v)
	{
		return v < u;
	}

	inline bool operator<=(Vector4 const& u, Vector4 const& v)
	{
		return !(v < u);
	}

	inline bool operator>=(Vector4 const& u, Vector4 const& v)
	{
		return !(u < v);
	}

	inline f32 dot(Vector4 const& u, Vector4 const& v)
	{
		__m128 mU = _mm_loadu_ps(u.m_values);
		__m128 mV = _mm_loadu_ps(v.m_values);
		__m128 mRes = _mm_dp_ps(mU, mV, 0xf1 /* mask */);

		// mask: 1111 0001 (use all 4 positions for dot product & save result in only the first position)
		// the high bits define the condition for dot product - only positions with 1 in the high bits are used
		// the low bits define the broadcast - broadcast the result to all positions with 1 in the low bits

		float res = _mm_cvtss_f32(mRes);
		return res;
	}

	inline Vector4 min(Vector4 const& u, Vector4 const& v)
	{
		return { apex::min(u.x, v.x), apex::min(u.y, v.y), apex::min(u.z, v.z), apex::min(u.w, v.w) };
	}

	inline Vector4 max(Vector4 const& u, Vector4 const& v)
	{
		return { apex::max(u.x, v.x), apex::max(u.y, v.y), apex::max(u.z, v.z), apex::max(u.w, v.w) };
	}

#pragma endregion

}
}
