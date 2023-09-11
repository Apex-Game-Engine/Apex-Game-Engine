#pragma once
#pragma message("Including Vector4.inl")

#include "Vector4.h"

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

	inline Vector4& Vector4::operator*=(const float32 t)
	{
		m_values[0] *= t;
		m_values[1] *= t;
		m_values[2] *= t;
		m_values[3] *= t;
		return *this;
	}

	inline Vector4& Vector4::operator/=(const float32 t)
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

	inline Vector4 operator*(float32 t, Vector4 const& v)
	{
		return { t * v.x, t * v.y, t * v.z, t * v.w };
	}

	inline Vector4 operator*(Vector4 const& v, float32 t)
	{
		return t * v;
	}

	inline Vector4 operator/(Vector4 const& v, float32 t)
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

#pragma endregion

}
}
