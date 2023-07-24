#pragma once
#include "Image.h"

struct HitRecord;
using apex::math::Ray;

/**
 * \brief Base class for all Material types
 */
struct Material
{
	virtual ~Material() = default;
	virtual bool scatter(Ray const &ray_in, HitRecord const &record, ColorRGB &out_attenuation, Ray &out_scattered) const = 0;
	[[nodiscard]] virtual ColorRGB emitted(Point3D const &point) const = 0;
};

namespace materials
{
	struct Lambertian : public Material
	{
		ColorRGB albedo { 0.5f };

		Lambertian() = default;
		Lambertian(ColorRGB const &albedo) : albedo(albedo) {}

		bool scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const override;
		[[nodiscard]] ColorRGB emitted(Point3D const& point) const override;
	};

	struct Metal : public Material
	{
		ColorRGB albedo { 0.5f };

		Metal() = default;
		Metal(ColorRGB const &albedo) : albedo(albedo) {}

		bool scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const override;
		[[nodiscard]] ColorRGB emitted(Point3D const& point) const override;
	};

	struct Emissive : public Material
	{
		ColorRGB emissive { 5.f };

		Emissive() = default;
		Emissive(ColorRGB const &emissive) : emissive(emissive) {}

		bool scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const override;
		[[nodiscard]] ColorRGB emitted(Point3D const& point) const override;
	};

	struct Glass : public Material
	{
		ColorRGB tint { 1.f };
		float32 refractiveIndex = 1.33f;

		Glass() = default;
		Glass(ColorRGB const &tint) : tint(tint) {}
		Glass(float32 refractive_index, ColorRGB const &tint) : tint(tint), refractiveIndex(refractive_index) {}

		bool scatter(Ray const& ray_in, HitRecord const& record, ColorRGB& out_attenuation, Ray& out_scattered) const override;
		[[nodiscard]] ColorRGB emitted(Point3D const& point) const override;
	};
}