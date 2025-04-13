#pragma once

#include "Quaternion.h"

#include "Vector3.inl"
#include "Matrix4x4.inl"

namespace apex {
namespace math {

	inline Quat Quat::fromAxisAngle(Vector3 const& axis, f32 angle)
	{
		Vector3 scaledAxis = std::sin(angle / 2) * axis.normalize();
		return Quat{ std::cos(angle / 2), scaledAxis.x, scaledAxis.y, scaledAxis.z };
	}

	inline auto Quat::fromEulerAngles(Vector3 const& euler_angles) -> Quat
	{
		f32 c1 = std::cos(euler_angles.x / 2);
		f32 s1 = std::sin(euler_angles.x / 2);
		f32 c2 = std::cos(euler_angles.y / 2);
		f32 s2 = std::sin(euler_angles.y / 2);
		f32 c3 = std::cos(euler_angles.z / 2);
		f32 s3 = std::sin(euler_angles.z / 2);

		return Quat{
			c1 * c2 * c3 + s1 * s2 * s3,
			s1 * c2 * c3 - c1 * s2 * s3,
			c1 * s2 * c3 + s1 * c2 * s3,
			c1 * c2 * s3 - s1 * s2 * c3
		};
	}

	inline auto Quat::xyz() const -> Vector3
	{
		return { x, y, z };
	}

	inline auto Quat::axis() const -> Vector3
	{
		f32 sinAngle = std::sqrt(1.0f - w * w);
		if (sinAngle < 0.0001f)
			return Vector3{ x, y, z };
		return Vector3{ x , y , z } / sinAngle;
	}

	inline auto Quat::angle() const -> f32
	{
		return 2 * std::acos(w);
	}

	inline auto Quat::matrix() const -> Matrix4x4
	{
		f32 xx = x * x;
		f32 yy = y * y;
		f32 zz = z * z;
		f32 xy = x * y;
		f32 xz = x * z;
		f32 yz = y * z;
		f32 wx = w * x;
		f32 wy = w * y;
		f32 wz = w * z;

		return Matrix4x4{
			1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0,
			2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0,
			2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0,
			0, 0, 0, 1
		};
	}

	inline auto Quat::conjugate() const -> Quat
	{
		return Quat{ w, -x, -y, -z };
	}

	inline auto Quat::inverse() const -> Quat
	{
		return conjugate() / lengthSquared();
	}

	inline auto Quat::applyToVector(Vector3 const& v) const -> Vector3
	{
		return (conjugate() * Quat(0, v) * (*this)).xyz();
	}

	inline auto Quat::dot(Quat const& a, Quat const& b) -> f32
	{
		return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline auto Quat::normalized() const -> Quat
	{
		return Quat{ w, x, y, z } / length();
	}

	inline auto Quat::length() const -> f32
	{
		return sqrt(lengthSquared());
	}

	inline auto Quat::lengthSquared() const -> f32
	{
		return dot(*this, *this);
	}

	inline Quat& Quat::operator+=(Quat const& v)
	{
		w += v.w;
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Quat& Quat::operator-=(Quat const& v)
	{
		w -= v.w;
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Quat& Quat::operator*=(Quat const& v)
	{
		f32 w1 = w * v.w - x * v.x - y * v.y - z * v.z;
		f32 x1 = w * v.x + x * v.w + y * v.z - z * v.y;
		f32 y1 = w * v.y - x * v.z + y * v.w + z * v.x;
		f32 z1 = w * v.z + x * v.y - y * v.x + z * v.w;
		w = w1;
		x = x1;
		y = y1;
		z = z1;
		return *this;
	}

	inline Quat& Quat::operator*=(f32 t)
	{
		w *= t;
		x *= t;
		y *= t;
		z *= t;
		return *this;
	}

	inline Quat& Quat::operator/=(f32 t)
	{
		w /= t;
		x /= t;
		y /= t;
		z /= t;
		return *this;
	}

	inline Quat operator+(Quat const& u, Quat const& v)
	{
		Quat u1(u);
		return u1 += v;
	}

	inline Quat operator-(Quat const& u, Quat const& v)
	{
		Quat u1(u);
		return u1 -= v;
	}

	inline Quat operator*(Quat const& u, Quat const& v)
	{
		Quat u1(u);
		return u1 *= v;
	}

	inline Quat operator*(f32 t, Quat const& v)
	{
		Quat v1(v);
		return v1 *= t;
	}

	inline Quat operator*(Quat const& v, f32 t)
	{
		Quat v1(v);
		return v1 *= t;
	}

	inline Quat operator/(Quat const& v, f32 t)
	{
		Quat v1(v);
		return v1 /= t;
	}

	inline bool operator==(Quat const& u, Quat const& v)
	{
		return u.w == v.w && u.x == v.x && u.y == v.y && u.z == v.z;
	}

	inline bool operator!=(Quat const& u, Quat const& v)
	{
		return !(u == v);
	}

	inline Quat slerp(Quat const& q1, Quat const& q2, f32 t)
	{
		f32 cosTheta = Quat::dot(q1, q2);
		if (cosTheta > 0.9995f)
		{
			Quat result = q1 + t * (q2 - q1);
			return result.normalized();
		}

		f32 theta = std::acos(cosTheta);
		f32 sinTheta = std::sin(theta);
		f32 t1 = std::sin((1 - t) * theta) / sinTheta;
		f32 t2 = std::sin(t * theta) / sinTheta;
		return q1 * t1 + q2 * t2;
	}
}
}
