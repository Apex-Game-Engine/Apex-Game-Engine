#pragma once
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"

namespace apex {
namespace gfx {

	struct CameraDesc
	{
		math::Vector3 position;
		math::Quat rotation;
		float fov = math::radians(60.f);
		float aspectRatio = 16.f/9.f;
		float nearZ = 0.1f;
		float farZ = 10000.f;
	};

	struct Camera {
		struct Matrices
		{
			math::Matrix4x4 view;				// World-space to View-space transformation
			math::Matrix4x4 projection;			// View-space to Clip-space transformation
			math::Matrix4x4 inverseView;		// View-space to World-space transformation
			math::Matrix4x4 inverseProjection;	// Clip-space to View-space transformation
		};
		Matrices matrices;

		void UpdateMatrices(const CameraDesc& camera)
		{
			static math::Matrix4x4 viewToVulkanTransformation = inverse(math::Matrix4x4{
				1, 0, 0, 0,
				0, -1, 0, 0,
				0, 0, -1, 0,
				0, 0, 0, 1 });
			math::Matrix4x4 cameraTransform = camera.rotation.matrix();
			cameraTransform.setTranslation(camera.position);
			matrices.inverseView = cameraTransform;
			matrices.view = cameraTransform.inverse();
			// Can precompute the projection matrix. But its okay for the demo.
			matrices.projection = math::perspective(camera.fov, camera.aspectRatio, camera.nearZ, camera.farZ) * viewToVulkanTransformation;
			// matrices.projection[1][1] *= -1;
			matrices.inverseProjection = inverse(matrices.projection);
		}
	};

}
}
