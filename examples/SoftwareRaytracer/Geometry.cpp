#include "Geometry.h"

bool Sphere::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	/** Ray-Sphere Intersection Test
	 *
	 * Ray : P(t) = O + t.d  |  O: origin, d: direction
	 * Sphere : (P(t) - C).(P(t) - C) = r^2 |  C: center, r: radiuss
	 *
	 *    => (O + t.d - C).(O + t.d - C) = r^2
	 *    => t^2.(d.d) + 2t.b.(O - C) + (O - C).(O - C) - r^2 = 0
	 *
	 * quadratic equation in `t`: a.t^2 + b.t + c = 0
	 *    => a = d.d ; b = 2b.(O - C) ; c = (O - C).(O - C) - r^2
	 */

	Vector3 OC = ray.origin - center;
	float32 a = dot(ray.direction, ray.direction);
	float32 half_b = dot(OC, ray.direction);
	float32 c = OC.length_squared() - radius*radius;

	float32 discriminant = half_b*half_b - a*c;
	if (discriminant < 0)
		return false;

	float32 sqrtd = apex::math::sqrt(discriminant);
	float32 root = (-half_b - sqrtd) / a;

	if (root < t_min || root > t_max)
	{
		root = (-half_b + sqrtd) / a;
		if (root < t_min || root > t_max)
		{
			return false;
		}
	}

	out_record.t = root;
	out_record.hitpoint = ray.at(root);
	out_record.pMaterial = pMaterial;
	Vector3 outwardNormal = (out_record.hitpoint - center) / radius;
	out_record.setFaceNormal(ray, outwardNormal);

	return true;
}

bool XYRect::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	const float32 k = constZ;
	const Point3D A = ray.origin;
	const Vector3 b = ray.direction;

	const float32 t = (k - A.z) / b.z;

	if (t < t_min || t > t_max)
	{
		return false;
	}

	const Point3D p = ray.at(t);

	if (minXY < p.xy() && p.xy() < maxXY)
	{
		out_record.t = t;
		out_record.hitpoint = p;
		out_record.setFaceNormal(ray, { 0.f, 0.f, 1.f });
		out_record.pMaterial = pMaterial;
		return true;
	}

	return false;
}

bool YZRect::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	const float32 k = constX;
	const Point3D A = ray.origin;
	const Vector3 b = ray.direction;

	const float32 t = (k - A.x) / b.x;

	if (t < t_min || t > t_max)
	{
		return false;
	}

	const Point3D p = ray.at(t);

	if (minYZ < p.yz() && p.yz() < maxYZ)
	{
		out_record.t = t;
		out_record.hitpoint = p;
		out_record.setFaceNormal(ray, { 1.f, 0.f, 0.f });
		out_record.pMaterial = pMaterial;
		return true;
	}

	return false;
}

bool XZRect::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	const float32 k = constY;
	const Point3D A = ray.origin;
	const Vector3 b = ray.direction;

	const float32 t = (k - A.y) / b.y;

	if (t < t_min || t > t_max)
	{
		return false;
	}

	const Point3D p = ray.at(t);

	if (minXZ < p.xz() && p.xz() < maxXZ)
	{
		out_record.t = t;
		out_record.hitpoint = p;
		out_record.setFaceNormal(ray, { 0.f, 1.f, 0.f });
		out_record.pMaterial = pMaterial;
		return true;
	}

	return false;
}

Box::Box(Point3D const& origin, Vector3 const& half_extents)
: origin(origin)
, halfExtents(half_extents)
{
	const Point3D minPt = origin - halfExtents;
	const Point3D maxPt = origin + halfExtents;

	XYRect frontFace;
	frontFace.constZ = minPt.z;
	frontFace.minXY = minPt.xy();
	frontFace.maxXY = maxPt.xy();

	XYRect backFace;
	backFace.constZ = maxPt.z;
	backFace.minXY = minPt.xy();
	backFace.maxXY = maxPt.xy();
}

bool Box::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	return _faces.checkHit(ray, t_min, t_max, out_record);
}

void RayTraceableShapeList::add(std::shared_ptr<RayTraceableShape>&& shape)
{
	m_objects.push_back(std::move(shape));
}

bool RayTraceableShapeList::checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const
{
	HitRecord tempRec;
	bool hitAnything = false;
	float32 closestSoFar = t_max;

	for (const auto& object : m_objects)
	{
		if (object->checkHit(ray, t_min, closestSoFar, tempRec))
		{
			hitAnything = true;
			closestSoFar = tempRec.t;
			out_record = tempRec;
		}
	}

	return hitAnything;
}
