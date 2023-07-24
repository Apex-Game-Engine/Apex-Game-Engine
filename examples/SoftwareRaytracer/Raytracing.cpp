#include "Raytracing.h"

#include "Material.h"
#include "Foundation/detail/Array.h"

void RayTracer::TraceRays(Camera const& camera, RayTraceableShapeList const& scene, int32 samples_per_pixel, int32 max_bounces)
{
	float32 avgScanlineDuration = 0.f;
	for (uint32 j = 0; j < m_image.height; ++j)
	{
		char msgbuf[256];
		int32 msgptr;

		auto startTime = std::chrono::high_resolution_clock::now();

		for (uint32 i = 0; i < m_image.width; ++i)
		{
			ColorRGB pixelColor{ 0.f, 0.f, 0.f };

			for (int32 s = 0; s < samples_per_pixel; s++)
			{
				float32 u = (i + Random::getFloat32()) / static_cast<float32>(m_image.width - 1);
				float32 v = (j + Random::getFloat32()) / static_cast<float32>(m_image.height - 1);

				Ray ray = camera.getRay(u, v);
				
				pixelColor += TraceSingleRay(ray, scene, max_bounces);
			}

			float32 scale = 1.f / (float32)samples_per_pixel;
			if (mode != eDebugRenderNormals)
			{
				pixelColor.r = math::sqrt(scale * pixelColor.r);
				pixelColor.g = math::sqrt(scale * pixelColor.g);
				pixelColor.b = math::sqrt(scale * pixelColor.b);
			}
			else
			{
				pixelColor *= scale;
			}

			m_image.set(i, j, pixelColor);
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		float32 scanlineDuration = std::chrono::duration<float32>(endTime - startTime).count();
		avgScanlineDuration = (avgScanlineDuration * j + scanlineDuration) / static_cast<float32>(j + 1);

		int32 scanlinesRemaining = m_image.height - j - 1;
		float32 eta = (float32)scanlinesRemaining * avgScanlineDuration;

		msgptr = sprintf_s(msgbuf, std::size(msgbuf), "\rScanlines remaining: %d", scanlinesRemaining);
		//msgptr += sprintf_s(msgbuf + msgptr, std::size(msgbuf) - msgptr, "| Time taken: %0.3f s", scanlineDuration);
		//msgptr += sprintf_s(msgbuf + msgptr, std::size(msgbuf) - msgptr, "| Time remaining: %0.3f s", eta);

		axDebug(msgbuf);
	}
}

ColorRGB RayTracer::TraceSingleRay(Ray const& ray, RayTraceableShapeList const& scene, int32 nbounces)
{
	if (nbounces <= 0)
		return { 0.f, 0.f, 0.f };

	HitRecord hitRecord;
	if (scene.checkHit(ray, 0.001f, constants::float32_INFINITY, hitRecord))
	{
		if (mode != eDebugRenderNormals)
		{
			Ray scattered;
			ColorRGB attenuation;
			ColorRGB emitted = hitRecord.pMaterial->emitted(hitRecord.hitpoint);

			if (hitRecord.pMaterial->scatter(ray, hitRecord, attenuation, scattered))
				return emitted + attenuation * TraceSingleRay(scattered, scene, nbounces - 1);

			return emitted;
		}
		else
		{
			return 0.5f * (hitRecord.normal + 1.f);
		}
	}
	return BackgroundColor(ray);
}

ColorRGB RayTracer::BackgroundColor(Ray const& ray) const
{
	const Vector3 unit_direction = normalize(ray.direction);
	const float32 t = 0.5f * (unit_direction.y + 1.f);
	return (1.f - t) * 0.1f * ColorRGB{ 0.3f, 0.1f, 0.03f } + t * 0.1f * ColorRGB{ 0.1f, 0.1f, 0.3f };
}




