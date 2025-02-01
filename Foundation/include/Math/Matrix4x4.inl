#pragma once
//#pragma message("Including Matrix4x4.inl")

#include "Matrix4x4.h"

#include "Vector3.inl"
#include "Vector4.inl"

namespace apex {
namespace math {


	/**************************************************************
	 * Matrix4x4
	 *************************************************************/

	#pragma region Matrix4x4 member functions

	inline Matrix4x4 Matrix4x4::transpose() const
	{
		return {
			m_columns[0].x, m_columns[1].x, m_columns[2].x, m_columns[3].x,
			m_columns[0].y, m_columns[1].y, m_columns[2].y, m_columns[3].y,
			m_columns[0].z, m_columns[1].z, m_columns[2].z, m_columns[3].z,
			m_columns[0].w, m_columns[1].w, m_columns[2].w, m_columns[3].w
		};
	}

	inline Matrix4x4 Matrix4x4::inverse() const
	{
		return math::inverse(*this);
	}

#pragma endregion

	#pragma region Matrix4x4 utility functions

	inline Matrix4x4 operator*(Matrix4x4 const& m, f32 t)
	{
		return {
			m[0] * t,
			m[1] * t,
			m[2] * t,
			m[3] * t
		};
	}

	inline Vector4 operator*(Matrix4x4 const &m, Vector4 const &v)
	{
		auto& c0 = m.m_columns[0];
		auto& c1 = m.m_columns[1];
		auto& c2 = m.m_columns[2];
		auto& c3 = m.m_columns[3];

		return {
			(c0.x * v.x) + (c1.x * v.y) + (c2.x * v.z) + (c3.x * v.w),
			(c0.y * v.x) + (c1.y * v.y) + (c2.y * v.z) + (c3.y * v.w),
			(c0.z * v.x) + (c1.z * v.y) + (c2.z * v.z) + (c3.z * v.w),
			(c0.w * v.x) + (c1.w * v.y) + (c2.w * v.z) + (c3.w * v.w)
		};
	}

	inline Matrix4x4 operator*(Matrix4x4 const& m1, Matrix4x4 const& m2)
	{
		Vector4 X = m1 * m2[0];
		Vector4 Y = m1 * m2[1];
		Vector4 Z = m1 * m2[2];
		Vector4 W = m1 * m2[3];

		return Matrix4x4{ X, Y, Z, W };
	}

	inline Matrix4x4 inverse(Matrix4x4 const& m)
	{
		float c00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		float c02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		float c03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
		float c04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		float c06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		float c07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
		float c08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		float c10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
        float c11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
		float c12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		float c14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		float c15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
		float c16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		float c18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		float c19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
		float c20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		float c22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		float c23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
		Vector4 fac0 { c00, c00, c02, c03 };
		Vector4 fac1 { c04, c04, c06, c07 };
		Vector4 fac2 { c08, c08, c10, c11 };
		Vector4 fac3 { c12, c12, c14, c15 };
		Vector4 fac4 { c16, c16, c18, c19 };
		Vector4 fac5 { c20, c20, c22, c23 };
		Vector4 vec0 { m[1][0], m[0][0], m[0][0], m[0][0] };
		Vector4 vec1 { m[1][1], m[0][1], m[0][1], m[0][1] };
		Vector4 vec2 { m[1][2], m[0][2], m[0][2], m[0][2] };
        Vector4 vec3 { m[1][3], m[0][3], m[0][3], m[0][3] };
		Vector4 inv0 { vec1 * fac0 - vec2 * fac1 + vec3 * fac2 };
		Vector4 inv1 { vec0 * fac0 - vec2 * fac3 + vec3 * fac4 };
		Vector4 inv2 { vec0 * fac1 - vec1 * fac3 + vec3 * fac5 };
		Vector4 inv3 { vec0 * fac2 - vec1 * fac4 + vec2 * fac5 };
		Vector4 signA { +1, -1, +1, -1 };
		Vector4 signB { -1, +1, -1, +1 };
		Matrix4x4 inverse { inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB };
		Vector4 row0 { inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0] };
		Vector4 dot0 { m[0] * row0 };
		f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);
		f32 one_over_determinant = 1.0f / dot1;
		return inverse * one_over_determinant;
	}

	inline Matrix4x4 rotateX(Matrix4x4 const& m, f32 angle)
	{
		f32 c = std::cos(angle);
		f32 s = std::sin(angle);

		/* x-axis rotation matrix :
		 *     1, 0, 0, 0,
		 *     0, c,-s, 0,
		 *     0, s, c, 0,
		 *     0, 0, 0, 1,
		 */

		Matrix4x4 res { row_major{},
			              m[0][0],               m[1][0],               m[2][0],               m[3][0],
			c*m[0][1] - s*m[0][2], c*m[1][1] - s*m[1][2], c*m[2][1] - s*m[2][2], c*m[3][1] - s*m[3][2],
			s*m[0][1] + c*m[0][2], s*m[1][1] + c*m[1][2], s*m[2][1] + c*m[2][2], s*m[3][1] + c*m[3][2],
			              m[0][3],               m[1][3],               m[2][3],               m[3][3],
		};

		return res;
	}

	inline Matrix4x4 rotateY(Matrix4x4 const& m, f32 angle)
	{
		f32 c = std::cos(angle);
		f32 s = std::sin(angle);

		/* x-axis rotation matrix :
		 *     c, 0, s, 0,
		 *     0, 1, 0, 0,
		 *    -s, 0, c, 0,
		 *     0, 0, 0, 1,
		 */

		Matrix4x4 res { row_major{},
			 c*m[0][0] + s*m[0][2],  c*m[1][0] + s*m[1][2],  c*m[2][0] + s*m[2][2],  c*m[3][0] + s*m[3][2],
			               m[0][1],                m[1][1],                m[2][1],                m[3][1],
			-s*m[0][0] + c*m[0][2], -s*m[1][0] + c*m[1][2], -s*m[2][0] + c*m[2][2], -s*m[3][0] + c*m[3][2],
			               m[0][3],                m[1][3],                m[2][3],                m[3][3],
		};

		return res;
	}

	inline Matrix4x4 rotateZ(Matrix4x4 const& m, f32 angle)
	{
		f32 c = std::cos(angle);
		f32 s = std::sin(angle);

		/* x-axis rotation matrix :
		 *     c,-s, 0, 0,
		 *     s, c, 0, 0,
		 *     0, 0, 1, 0,
		 *     0, 0, 0, 1,
		 */

		Matrix4x4 res { row_major{},
			c*m[0][0] - s*m[0][1], c*m[1][0] - s*m[1][1], c*m[2][0] - s*m[2][1], c*m[3][0] - s*m[3][1],
			s*m[0][0] + c*m[0][1], s*m[1][0] + c*m[1][1], s*m[2][0] + c*m[2][1], s*m[3][0] + c*m[3][1],
			              m[0][2],               m[1][2],               m[2][2],               m[3][2],
			              m[0][3],               m[1][3],               m[2][3],               m[3][3],
		};

		return res;
	}

	inline Matrix4x4 rotateZYX(Matrix4x4 const& m, f32 angleX, f32 angleY, f32 angleZ)
	{
		return eulerZYX(angleX, angleY, angleZ) * m;
	}

	inline Matrix4x4 rotateZYX(Matrix4x4 const& m, Vector3 const& angles)
	{
		return rotateZYX(m, angles.x, angles.y, angles.z);
	}

	inline Matrix4x4 rotateAxisAngle(Matrix4x4 const& m, Vector3 const& axis, f32 angle)
	{
		return rotateAxisAngle(m, axis, std::cos(angle), std::sin(angle));
	}

	inline Matrix4x4 rotateAxisAngle(Matrix4x4 const& m, Vector3 const& axis, f32 cosAngle, f32 sinAngle)
	{
		f32 c = cosAngle;
		f32 s = sinAngle;
		f32 t = 1.f - c;
		Vector3 a = normalize(axis);
		f32 x = a.x;
		f32 y = a.y;
		f32 z = a.z;
		Matrix4x4 res { row_major{},
			t*x*x + c, t*x*y - s*z, t*x*z + s*y, 0,
			t*x*y + s*z, t*y*y + c, t*y*z - s*x, 0,
			t*x*z - s*y, t*y*z + s*x, t*z*z + c, 0,
			0, 0, 0, 1
		};
		return res * m;
	}

	inline Vector3 decomposeRotation(Matrix4x4 const& m)
	{
		f32 x, y, z;
		if (m[0][2] < 1.f)
		{
			if (m[0][2] > -1.f)
			{
				y = std::asin(-m[0][2]);
				z = std::atan2(m[0][1], m[0][0]);
				x = std::atan2(m[1][2], m[2][2]);
			}
			else
			{
				// Not a unique solution: x - z = atan2(-m[2][1], m[1][1])
				y = Constants::f32_PI / 2;
				z = -std::atan2(-m[2][1], m[1][1]);
				x = 0;
			}
		}
		else
		{
			// Not a unique solution: x + z = atan2(-m[2][1], m[1][1])
			y = -Constants::f32_PI;
			z = std::atan2(-m[2][1], m[1][1]);
			x = 0;
		}
		return {x, y, z};
	}

	inline Matrix4x4 translate(Matrix4x4 const& m, Vector3 const& v)
	{
		Matrix4x4 res {
			m[0], m[1], m[2], m[3] + Vector4{ v, 0.f }
		};

		return res;
	}

	inline Matrix4x4 scale(Matrix4x4 const& m, Vector3 const& v)
	{
		Matrix4x4 res {
			m[0] * v.x,
			m[1] * v.y,
			m[2] * v.z,
			m[3]
		};

		return res;
	}

	inline Matrix4x4 lookAt_slow(Vector3 eye, Vector3 target, Vector3 up)
	{
		Vector3 Z = normalize(eye - target);
		Vector3 X = normalize(cross(up, Z));
		Vector3 Y = cross(Z, X);

		// Transpose (inverse) of the rotation matrix
		Matrix4x4 Rot { row_major{},
			X.x, X.y, X.z, 0,
			Y.x, Y.y, Y.z, 0,
			Z.x, Z.y, Z.z, 0,
			  0,   0,   0, 1
		};

		// Inverse of the translation matrix. T(v)^-1 = T(-v)
		Matrix4x4 Trn {
			Vector4::unitX(),
			Vector4::unitY(),
			Vector4::unitZ(),
			{ -eye, 1.f }
		};

		// Inverse of camera transformation matrix
		// View = (TR)^-1 = (R^-1)(T^-1)
		return Rot * Trn;
	}

	inline Matrix4x4 lookAt(Vector3 eye, Vector3 target, Vector3 up)
	{
		Vector3 Z = normalize(eye - target);
		Vector3 X = normalize(cross(up, Z));
		Vector3 Y = cross(Z, X);

		// Transpose (inverse) of the rotation matrix
		Matrix4x4 View { row_major{},
			X.x, X.y, X.z, -dot(X, eye),
			Y.x, Y.y, Y.z, -dot(Y, eye),
			Z.x, Z.y, Z.z, -dot(Z, eye),
			  0,   0,   0, 1
		};

		return View;
	}

	inline Matrix4x4 perspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
	{
		f32 recip_tan_fov_by_2 = cos(fov * 0.5f) / sin(fov * 0.5f);
		f32 z_range = far_z - near_z;

		Matrix4x4 Proj { row_major{},
			recip_tan_fov_by_2 / aspect_ratio,          0        ,              0              ,             0                ,
			              0                  , recip_tan_fov_by_2,              0              ,             0                ,
			              0                  ,          0        , -(near_z + far_z) / z_range , -2 * near_z * far_z / z_range,
			              0                  ,          0        ,             -1              ,             0
		};

		return Proj;
	}

	inline Matrix4x4 eulerZYX(f32 angleX, f32 angleY, f32 angleZ)
	{
		f32 A = angleX;
		f32 B = angleY;
		f32 Y = angleZ;

		f32 cA = cos(A), sA = sin(A);
		f32 cB = cos(B), sB = sin(B);
		f32 cY = cos(Y), sY = sin(Y);

		f32 cAcB = cA * cB;
		f32 sAcB = sA * cB;

		f32 cAcY = cA * cY;
		f32 cAsY = cA * sY;
		f32 sAcY = sA * cY;
		f32 sAsY = sA * sY;

		f32 cBcY = cB * cY;
		f32 cBsY = cB * sY;

		Matrix4x4 rot { row_major{},
			cBcY, sB * sAcY - cAsY, sB * cAcY + sAsY, 0,
			cBsY, sB * sAsY + cAcY, sB * cAsY - sAcY, 0,
			 -sB,             sAcB,             cAcB, 0,
			   0,                0,                0, 1
		};

		return rot;
	}

#pragma endregion

}	
}
