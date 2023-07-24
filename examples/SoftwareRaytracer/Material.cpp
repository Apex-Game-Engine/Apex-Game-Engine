#include "Material.h"

#include "Geometry.h"


bool materials::Lambertian::scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, 	Ray& out_scattered) const
{
	Vector3 rayDirection = ray_in.direction.normalize();
	float32 cosTheta = fmin(dot(rayDirection, record.normal), 1.f);

	Vector3 scatterDirection = record.normal + Vector3::random_unit_vector();
	// Catch degenerate scatter direction
	scatterDirection = scatterDirection.is_near_zero() ? record.normal : scatterDirection;

	out_scattered = Ray{ record.hitpoint, scatterDirection };
	out_attenuation = albedo;

	return true;
}

ColorRGB materials::Lambertian::emitted(Point3D const& point) const
{
	return {};
}

bool materials::Metal::scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const
{
	Vector3 reflected = reflect(ray_in.direction.normalize(), record.normal);
	out_scattered = Ray{ record.hitpoint, reflected };
	out_attenuation = albedo;
	return dot(out_scattered.direction, record.normal) > 0;
}

ColorRGB materials::Metal::emitted(Point3D const& point) const
{
	return {};
}

bool materials::Emissive::scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const
{
	return false;
}

ColorRGB materials::Emissive::emitted(Point3D const& point) const
{
	return emissive;
}

bool materials::Glass::scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const
{
	float32 refractionRatio = record.frontFace ? (1.f / refractiveIndex) : refractiveIndex;

	Vector3 rayDirection = ray_in.direction.normalize();
	float32 cosTheta = fmin(dot(-rayDirection, record.normal), 1.f);
	float32 sinTheta = math::sqrt(1.f - cosTheta * cosTheta);

	bool totalInternalReflection = refractionRatio * sinTheta > 1.f;

	float32 randomSample = Random::getFloat32();
	bool fresnelReflection = record.frontFace && (randomSample > cosTheta) && false;

	Vector3 newDirection;

	if (totalInternalReflection || fresnelReflection)
		newDirection = reflect(rayDirection, record.normal);
	else
		newDirection = refract(rayDirection, record.normal, refractionRatio);

	out_scattered = Ray{ record.hitpoint, newDirection };
	out_attenuation = tint;

	return true;
}

ColorRGB materials::Glass::emitted(Point3D const& point) const
{
	return {};
}
