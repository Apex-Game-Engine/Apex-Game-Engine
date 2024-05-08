#pragma once
//#pragma message("Including Vector2.inl")

#include "Vector2.h"
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

	inline Vector2& Vector2::operator/=(Vector2 const& v)
	{
		return (*this) *= { 1/v.x, 1/v.y };
	}

	inline Vector2& Vector2::operator/=(float32 t)
	{
		return (*this) *= 1/t;
	}

	inline float32 Vector2::length() const
	{
		return apex::math::sqrt(lengthSquared());
	}

	inline float32 Vector2::lengthSquared() const
	{
		return x*x + y*y;
	}

	inline Vector2 Vector2::normalize() const
	{
		return apex::math::normalize(*this);
	}

	inline bool Vector2::isNearZero() const
	{
		return floatCompareNearZero(m_values[0])
			&& floatCompareNearZero(m_values[1]);
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

	inline Vector2 operator/(Vector2 const& u, Vector2 const& v)
	{
		Vector2 u1(u);
		return u1 /= v;
	}

	inline bool operator==(Vector2 const& u, Vector2 const& v)
	{
		return floatCompareApproximatelyEqual(u.x, v.x)
			&& floatCompareApproximatelyEqual(u.y, v.y);
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

}
}
