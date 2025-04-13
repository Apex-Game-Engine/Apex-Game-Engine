#pragma once
#include "Core/Types.h"

namespace apex {
namespace math {
	struct Matrix4x4;
	struct Vector3;

	struct Quat
	{
		// q = w + xi + yj + zk
		f32 w = 1.0f;
		f32 x = 0.0f;
		f32 y = 0.0f;
		f32 z = 0.0f;

		constexpr Quat() = default;
		constexpr Quat(f32 w, f32 x, f32 y, f32 z) : w(w), x(x), y(y), z(z) {}
		constexpr Quat(f32 w, Vector3 const& xyz) : w(w), x(xyz.x), y(xyz.y), z(xyz.z) {}

		// Constructs a quaternion from an axis and an angle
		static auto fromAxisAngle(Vector3 const& axis, f32 angle) -> Quat;

		// Constructs a quaternion from Euler angles
		static auto fromEulerAngles(Vector3 const& euler_angles) -> Quat;

		// Constructs a quaternion from a rotation matrix
		static auto fromMatrix(Matrix4x4 const& matrix) -> Quat;

		auto xyz() const -> Vector3;

		// Returns the axis of the quaternion
		auto axis() const -> Vector3;

		// Returns the angle of the quaternion
		auto angle() const -> f32;

		// Returns the Euler angles of the quaternion
		auto eulerAngles() const -> Vector3;

		// Returns the rotation matrix of the quaternion
		auto matrix() const -> Matrix4x4;

		// Returns the conjugate of the quaternion
		auto conjugate() const -> Quat;

		// Returns the inverse of the quaternion
		auto inverse() const -> Quat;

		// Applies the quaternion rotation to the vector 
		Vector3 applyToVector(Vector3 const& v) const;

		// Returns the dot product of two quaternions
		static auto dot(Quat const& a, Quat const& b) -> f32;

		// Returns the normalized quaternion
		auto normalized() const -> Quat;

		// Returns the length of the quaternion
		auto length() const -> f32;

		// Returns the squared length of the quaternion
		auto lengthSquared() const -> f32;

		Quat & operator+=(Quat const &v);
		Quat & operator-=(Quat const &v);
		Quat & operator*=(Quat const &v);
		Quat & operator*=(f32 t);
		Quat & operator/=(f32 t);
	};

	// Quat utility functions

	Quat operator+(Quat const &u, Quat const &v); // addition of two quaternions
	Quat operator-(Quat const &u, Quat const &v); // subtraction of two quaternions
	Quat operator*(Quat const &u, Quat const &v); // multiplication of two quaternions
	Quat operator*(f32 t, Quat const &v); // scalar multiplication of quaternion
	Quat operator*(Quat const &v, f32 t); // scalar multiplication of quaternion
	Quat operator/(Quat const &v, f32 t); // scalar division of quaternion

	bool operator==(Quat const &u, Quat const &v); // element-wise comparison
	bool operator!=(Quat const &u, Quat const &v); // element-wise comparison

	Quat slerp(Quat const &q1, Quat const &q2, f32 t); // spherical linear interpolation

}
}


#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Quaternion.inl"
#endif

