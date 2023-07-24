#pragma once
#include "Foundation/detail/Vector3.h"

using apex::float32;
using apex::math::Vector3;
using apex::math::Point3D;
using apex::math::Ray;

struct CameraParams
{
	Point3D origin;
	float32 viewport_height;
	float32 viewport_width;
	float32 focal_length;
};

struct Camera
{
	explicit Camera(CameraParams const &params);
	Camera(Point3D origin, float32 viewport_width, float32 viewport_height, float32 focal_length);

	Ray getRay(float32 u, float v) const;

	Point3D origin;
	Vector3 right;
	Vector3 up;
	Point3D lower_left_corner;
};

