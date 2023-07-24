#pragma once
#include <memory>
#include <vector>

#include "Foundation/axMath.h"

using apex::float32;
using apex::math::Vector2;
using apex::math::Vector3;
using apex::math::Point3D;
using apex::math::Ray;

struct Material;

struct HitRecord
{
	Point3D hitpoint;
	Vector3 normal;
	std::shared_ptr<Material> pMaterial;
	float32 t; // parameter of ray parametric form
	bool frontFace;

	void setFaceNormal(Ray const &ray, Vector3 const &outward_normal)
	{
		frontFace = dot(ray.direction, outward_normal) < 0;
		normal = frontFace ? outward_normal : -outward_normal;
	}
};

struct RayTraceableShape
{
	virtual ~RayTraceableShape() = default;
	virtual bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord &out_record) const = 0;
};

struct RayTraceableShapeList : public RayTraceableShape
{
public:
	RayTraceableShapeList() = default;
	~RayTraceableShapeList() override = default;

	using ShapeList = std::vector<std::shared_ptr<RayTraceableShape>>;

	void add(std::shared_ptr<RayTraceableShape>&& shape);

	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord &out_record) const override;

	//ShapeList::iterator begin() { return m_objects.begin(); }
	//ShapeList::iterator end() { return m_objects.end(); }

	ShapeList::const_iterator begin() const { return m_objects.begin(); }
	ShapeList::const_iterator end() const { return m_objects.end(); }

private:
	 ShapeList m_objects{};
};

struct Sphere : public RayTraceableShape
{
	Point3D center;
	float32 radius;
	std::shared_ptr<Material> pMaterial;
	
	~Sphere() override = default;
	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const override;
};

struct XYRect : public RayTraceableShape
{
	Vector2 minXY;
	Vector2 maxXY;
	float32 constZ;
	std::shared_ptr<Material> pMaterial;

	~XYRect() override = default;
	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const override;
};

struct YZRect : public RayTraceableShape
{
	Vector2 minYZ;
	Vector2 maxYZ;
	float32 constX;
	std::shared_ptr<Material> pMaterial;

	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const override;
};

struct XZRect : public RayTraceableShape
{
	Vector2 minXZ;
	Vector2 maxXZ;
	float32 constY;
	std::shared_ptr<Material> pMaterial;

	~XZRect() override = default;
	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const override;
};

struct Box : public RayTraceableShape
{
	Point3D origin;
	Vector3 halfExtents;
	std::shared_ptr<Material> pMaterial;

	Box(Point3D const &origin, Vector3 const &half_extents);
	~Box() override = default;

	Point3D minimum() const;
	Point3D maximum() const;

	bool checkHit(Ray const& ray, float32 t_min, float32 t_max, HitRecord& out_record) const override;

protected:
	RayTraceableShapeList _faces;
};
