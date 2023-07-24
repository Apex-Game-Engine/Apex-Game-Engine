#include "Camera.h"
#include "Geometry.h"

Camera::Camera(CameraParams const& params)
: Camera(params.origin, params.viewport_width, params.viewport_height, params.focal_length)
{
}

Camera::Camera(Point3D origin, float32 viewport_width, float32 viewport_height, float32 focal_length)
: origin(origin)
, right{ viewport_width, 0.f, 0.f }
, up{ 0.f, viewport_height, 0.f }
, lower_left_corner{ origin - right/2 - up/2 - Vector3{ 0.f, 0.f, focal_length } }
{
}

Ray Camera::getRay(float32 u, float v) const
{
	return Ray{ origin, lower_left_corner + u * right + v * up - origin };
}
