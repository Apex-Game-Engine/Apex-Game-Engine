#pragma once
#include "Core/Types.h"

#ifndef APEX_MATH_SKIP_INLINE_IMPL
#	define APEX_MATH_SKIP_INLINE_IMPL
#	include "Vector4.h"
#	undef APEX_MATH_SKIP_INLINE_IMPL
#else
#	include "Vector4.h"
#endif

namespace apex {
namespace math {

	struct column_major{};
	struct row_major{};

	// Column-major 4x4 matrix. Compliant with GLSL mat4
	struct Matrix4x4
	{
		using col_type = Vector4;
		using row_type = Vector4;
		
		Vector4 m_columns[4] {};

		constexpr Matrix4x4() = default;

		// Constructs a 4x4 matrix with given values
		constexpr Matrix4x4(
			f32 x0, f32 y0, f32 z0, f32 w0,
			f32 x1, f32 y1, f32 z1, f32 w1,
			f32 x2, f32 y2, f32 z2, f32 w2,
			f32 x3, f32 y3, f32 z3, f32 w3)
		: m_columns { 
			{ x0, y0, z0, w0 },
			{ x1, y1, z1, w1 },
			{ x2, y2, z2, w2 },
			{ x3, y3, z3, w3 }
		}
		{}

		constexpr Matrix4x4(row_major,
			f32 x0, f32 x1, f32 x2, f32 x3,
			f32 y0, f32 y1, f32 y2, f32 y3,
			f32 z0, f32 z1, f32 z2, f32 z3,
			f32 w0, f32 w1, f32 w2, f32 w3)
		: m_columns { 
			{ x0, y0, z0, w0 },
			{ x1, y1, z1, w1 },
			{ x2, y2, z2, w2 },
			{ x3, y3, z3, w3 }
		}
		{}

		// Constructs a 4x4 matrix with given column vectors
		constexpr Matrix4x4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3)
		: m_columns { c0, c1, c2, c3 }
		{}

		// Constructs an identity matrix scaled by given diagonal value
		explicit constexpr Matrix4x4(f32 diagonal_value)
		: m_columns {
			{ diagonal_value, 0.f, 0.f, 0.f },
			{ 0.f, diagonal_value, 0.f, 0.f },
			{ 0.f, 0.f, diagonal_value, 0.f },
			{ 0.f, 0.f, 0.f, diagonal_value }
		}
		{}

		explicit constexpr Matrix4x4(f32 d0, f32 d1, f32 d2, f32 d3)
		: m_columns {
			{ d0, 0.f, 0.f, 0.f },
			{ 0.f, d1, 0.f, 0.f },
			{ 0.f, 0.f, d2, 0.f },
			{ 0.f, 0.f, 0.f, d3 }
		}
		{}

		Vector4 operator [](size_t index) const { return m_columns[index]; }
		Vector4& operator [](size_t index) { return m_columns[index]; }

		Matrix4x4 transpose() const;
		Matrix4x4 inverse() const;

		static constexpr Matrix4x4 identity()
		{
			return Matrix4x4(1.f);
		}

		Vector3 getTranslation() const;
		void setTranslation(Vector3 const& v);
		//Matrix3x3 getRotation() const;
	};

	static_assert(sizeof(Matrix4x4) == 16 * sizeof(float));

	// Matrix4x4 utility functions

	Matrix4x4 operator*(Matrix4x4 const &m, f32 t); // multiply each element of matrix with scalar
	Vector4 operator*(Matrix4x4 const &m, Vector4 const &v); // matrix-vector multiplication
	Matrix4x4 operator*(Matrix4x4 const &m1, Matrix4x4 const &m2); // matrix-vector multiplication

	Matrix4x4 inverse(Matrix4x4 const &m); // matrix inverse

	Matrix4x4 rotateX(Matrix4x4 const &m, f32 angle); // rotation about the x-axis
	Matrix4x4 rotateY(Matrix4x4 const &m, f32 angle); // rotation about the y-axis
	Matrix4x4 rotateZ(Matrix4x4 const &m, f32 angle); // rotation about the z-axis

	// rotation about the euler angles in Rz*Ry*Rx order
	// equivalent to rotateZ(rotateY(rotateX(m, angleX), angleY), angleZ)
	Matrix4x4 rotateZYX(Matrix4x4 const &m, f32 angleX, f32 angleY, f32 angleZ);
	Matrix4x4 rotateZYX(Matrix4x4 const &m, Vector3 const& angles);

	Matrix4x4 rotateAxisAngle(Matrix4x4 const &m, Vector3 const &axis, f32 angle); // rotation about an arbitrary axis
	Matrix4x4 rotateAxisAngle(Matrix4x4 const& m, Vector3 const& axis, f32 cosAngle, f32 sinAngle); // rotation about an arbitrary axis

	Vector3 decomposeRotation(Matrix4x4 const &m); // extract euler angles from rotation matrix

	Matrix4x4 translate(Matrix4x4 const &m, Vector3 const &v); // translation by vector v

	Matrix4x4 scale(Matrix4x4 const &m, Vector3 const &v); // scale by vector v

	Matrix4x4 lookAt(Vector3 eye, Vector3 target, Vector3 up); // look-at matrix for camera facing in -Z direction

	Matrix4x4 perspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z); // perspective projection

	// rotation by euler angles (as a compound matrix transformation)
	// equivalent to rotateZ(rotateY(rotateX(m, angleX), angleY), angleZ)
	Matrix4x4 eulerZYX(f32 angleX, f32 angleY, f32 angleZ);

}
}

#ifndef APEX_MATH_SKIP_INLINE_IMPL
#include "Matrix4x4.inl"
#endif
