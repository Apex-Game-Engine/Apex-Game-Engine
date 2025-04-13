#include <gtest/gtest.h>

#include "Math/Math.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"

namespace apex::math {

	TEST(TestVector3, TestDotProduct)
	{
		Vector3 v1 { 1.f, 2.f, 3.f };
		Vector3 v2 { 4.f, 5.f, 6.f };

		float res = dot(v1, v2);
		EXPECT_FLOAT_EQ(res, 32.f);
	}

	TEST(TestVector3, TestCrossProduct)
	{
		Vector3 v1 { 1.f, 2.f, 3.f };
		Vector3 v2 { 4.f, 5.f, 6.f };

		Vector3 res = cross(v1, v2);
		Vector3 vres { -3.f, 6.f, -3.f };

		EXPECT_TRUE(res == vres);
	}

	TEST(TestMatrix4x4, TestMatrix)
	{
		{
			Matrix4x4 mat{};
			EXPECT_FLOAT_EQ(mat[0][0], 0.f);
		}

		{
			Matrix4x4 m {
				1, 2, 3, 4,
				5, 6, 7, 8,
				9, 10, 11, 12,
				13, 14, 15, 16
			};

			Matrix4x4 m2 = m.transpose();

			for (int c = 0; c < 4; c++)
			{
				for (int r = 0; r < 4; r++)
				{
					printf("%3.f,", m[c][r]);
					EXPECT_EQ(m[c][r], c * 4 + r + 1);
					EXPECT_EQ(m[c][r], m2[r][c]);
				}
				printf("\n");
			}
		}
	}

	TEST(TestMatrix4x4, TestMatrixVectorMultiplication)
	{
		{
			Vector4 v1 { 1.f, 2.f, 3.f, 4.f };
			Vector4 v2 = Matrix4x4::identity() * v1;

			EXPECT_TRUE(v1 == v2);
		}

		{
			Vector4 v1 { 1.f, 2.f, 3.f, 4.f };
			Matrix4x4 m {
				 0.f, -1.f,  0.f,  1.f,
	 			 1.f,  0.f, -1.f,  0.f,
				 0.f,  1.f,  0.f, -1.f,
				-1.f,  0.f,  1.f,  0.f
			};

			Vector4 v2 = m * v1;
			Vector4 vres { -2.f, 2.f, 2.f, -2.f };

			EXPECT_TRUE(v2 == vres);
		}

		{
			Vector4 v1 { 1.f, 2.f, 3.f, 4.f };
			Matrix4x4 m { row_major{},
				 0.f, -1.f,  0.f,  1.f,
	 			 1.f,  0.f, -1.f,  0.f,
				 0.f,  1.f,  0.f, -1.f,
				-1.f,  0.f,  1.f,  0.f
			};

			Vector4 v2 = m * v1;
			Vector4 vres { 2.f, -2.f, -2.f, 2.f };

			EXPECT_TRUE(v2 == vres);
		}
	}

	TEST(TestMatrix4x4, TestRotation)
	{
		{
			Vector4 vy = Vector4::unitY_w1();
			Matrix4x4 mrot = rotateX(Matrix4x4::identity(), radians(90.f));

			Vector4 vrot = mrot * vy;
			Vector4 vres = Vector4::unitZ_w1();

			EXPECT_TRUE(vrot == vres);

			printf("vrot (%0.2f, %0.2f, %0.2f, %0.2f) | vres (%0.2f, %0.2f, %0.2f, %0.2f)\n", vrot.x, vrot.y, vrot.z, vrot.w, vres.x, vres.y, vres.z, vres.w);
		}

		{
			Vector4 vx = Vector4::unitX_w1();
			Matrix4x4 mrot = rotateY(Matrix4x4::identity(), radians(-90.f));

			Vector4 vrot = mrot * vx;
			Vector4 vres = Vector4::unitZ_w1();

			EXPECT_TRUE(vrot == vres);

			printf("vrot (%0.2f, %0.2f, %0.2f, %0.2f) | vres (%0.2f, %0.2f, %0.2f, %0.2f)\n", vrot.x, vrot.y, vrot.z, vrot.w, vres.x, vres.y, vres.z, vres.w);
		}

		{
			Vector4 vx = Vector4::unitX_w1();
			Matrix4x4 mrot = rotateZ(Matrix4x4::identity(), radians(90.f));

			Vector4 vrot = mrot * vx;
			Vector4 vres = Vector4::unitY_w1();

			EXPECT_TRUE(vrot == vres);

			printf("vrot (%0.2f, %0.2f, %0.2f, %0.2f) | vres (%0.2f, %0.2f, %0.2f, %0.2f)\n", vrot.x, vrot.y, vrot.z, vrot.w, vres.x, vres.y, vres.z, vres.w);
		}
	}

	TEST(TestMatrix4x4, TestTranslation)
	{
		{
			Matrix4x4 mtrans = translate(Matrix4x4::identity(), Vector3{ 1.f, 2.f, 3.f });

			Vector4 v1 = Vector4::unitW();

			Vector4 vtrans = mtrans * v1;
			Vector4 vres { 1, 2, 3, 1 };

			EXPECT_TRUE(vtrans == vres);

			printf("vtrans (%0.2f, %0.2f, %0.2f, %0.2f) | vres (%0.2f, %0.2f, %0.2f, %0.2f)\n", vtrans.x, vtrans.y, vtrans.z, vtrans.w, vres.x, vres.y, vres.z, vres.w);
		}
	}

	TEST(TestMatrix4x4, TestViewProjection)
	{
		{
			Vector3 eye = { 0.f, 0.f, 0.f };
			Vector3 target = -math::Vector3::unitZ();
			Vector3 up = Vector3::unitY();

			Matrix4x4 view1 = math::lookAt(eye, target, up);
			Matrix4x4 view2 = math::lookAt_slow(eye, target, up);

			EXPECT_TRUE(view1[0] == view2[0]);
			EXPECT_TRUE(view1[1] == view2[1]);
			EXPECT_TRUE(view1[2] == view2[2]);
			EXPECT_TRUE(view1[3] == view2[3]);

			printf("%0.8f %0.8f %0.8f %0.8f\n", view1[0][0], view1[1][0], view1[2][0], view1[3][0]);
			printf("%0.8f %0.8f %0.8f %0.8f\n", view1[0][1], view1[1][1], view1[2][1], view1[3][1]);
			printf("%0.8f %0.8f %0.8f %0.8f\n", view1[0][2], view1[1][2], view1[2][2], view1[3][2]);
			printf("%0.8f %0.8f %0.8f %0.8f\n", view1[0][3], view1[1][3], view1[2][3], view1[3][3]);
		}
	}

	TEST(TestMatrix4x4, TestDecomposeRotation)
	{
		{
			Matrix4x4 mrot = eulerZYX(radians(105), radians(30), radians(25));

			Vector3 euler = decomposeRotation(mrot);
			Vector3 eulerres { radians(105), radians(30), radians(25) };

			EXPECT_TRUE(euler == eulerres);

			printf("euler (%0.2f, %0.2f, %0.2f) | eulerres (%0.2f, %0.2f, %0.2f)\n", euler.x, euler.y, euler.z, eulerres.x, eulerres.y, eulerres.z);
		}
	}

	TEST(TestQuat, TestHamiltonianProduct)
	{
		{
			Quat q1;
			Quat qrot = q1.fromAxisAngle(Vector3::unitX(), radians(90)); // 90 deg rotation about x-axis
			Quat q2 = qrot * q1;
			Quat q3 = qrot * q2;

			printf("q1 (%0.3f, %0.3f, %0.3f, %0.3f) | qrot (%0.3f, %0.3f, %0.3f, %0.3f) | q2 (%0.3f, %0.3f, %0.3f, %0.3f) | q3 (%0.3f, %0.3f, %0.3f, %0.3f)", q1.w, q1.x, q1.y, q1.z, qrot.w, qrot.x, qrot.y, qrot.z, q2.w, q2.x, q2.y, q2.z, q3.w, q3.x, q3.y, q3.z);
		}
	}

	TEST(TestQuat, TestApplyToVector)
	{
		{
			Vector3 pos { 0, 0, -1 };
			Quat q1 = Quat::fromAxisAngle({1, 0, 0}, radians(90));

			q1.applyToVector(pos);

			printf("pos (%0.3f, %0.3f, %0.3f)", pos.x, pos.y, pos.z);
		}
	}

}
