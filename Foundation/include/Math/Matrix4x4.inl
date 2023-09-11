#pragma once
#pragma message("Including Matrix4x4.inl")

#include "Matrix4x4.h"

namespace apex {
namespace math {


	/**************************************************************
	 * Matrix4x4
	 *************************************************************/

	#pragma region Matrix4x4 member functions

	Matrix4x4 Matrix4x4::transpose() const
	{
		return {
			m_columns[0].x, m_columns[1].x, m_columns[2].x, m_columns[3].x,
			m_columns[0].y, m_columns[1].y, m_columns[2].y, m_columns[3].y,
			m_columns[0].z, m_columns[1].z, m_columns[2].z, m_columns[3].z,
			m_columns[0].w, m_columns[1].w, m_columns[2].w, m_columns[3].w
		};
	}

#pragma endregion

	#pragma region Matrix4x4 utility functions

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

	Matrix4x4 rotateX(Matrix4x4 const& m, float32 angle)
	{
		float32 c = std::cos(angle);
		float32 s = std::sin(angle);

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

	inline Matrix4x4 rotateY(Matrix4x4 const& m, float32 angle)
	{
		float32 c = std::cos(angle);
		float32 s = std::sin(angle);

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

	inline Matrix4x4 rotateZ(Matrix4x4 const& m, float32 angle)
	{
		float32 c = std::cos(angle);
		float32 s = std::sin(angle);

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

	inline Matrix4x4 generateEulerMatrix(float32 angleX, float32 angleY, float32 angleZ)
	{
		float32 A = angleX;
		float32 B = angleY;
		float32 Y = angleZ;

		float32 cA = cos(A), sA = sin(A);
		float32 cB = cos(B), sB = sin(B);
		float32 cY = cos(Y), sY = sin(Y);

		float32 cAcB = cA * cB;
		float32 sAcB = sA * cB;

		float32 cAcY = cA * cY;
		float32 cAsY = cA * sY;
		float32 sAcY = sA * cY;
		float32 sAsY = sA * sY;

		float32 cBcY = cB * cY;
		float32 cBsY = cB * sY;

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
