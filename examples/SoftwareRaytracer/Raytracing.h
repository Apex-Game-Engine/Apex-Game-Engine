#pragma once

#include "Camera.h"
#include "Geometry.h"
#include "Image.h"

class RayTracer
{
public:
	enum RenderMode
	{
		eFull,
		eDebugRenderNormals
	};

public:
	RayTracer(uint32 width, uint32 height)
	: m_image{width, height, new ColorRGB[width * height]{} }
	{}

	void TraceRays(Camera const& camera, RayTraceableShapeList const& scene, int32 samples_per_pixel, int32 max_bounces);
	Image const& GetImage() { return m_image; }

	RenderMode mode { eFull };

protected:
	ColorRGB TraceSingleRay(Ray const& ray, RayTraceableShapeList const& scene, int32 nbounces);
	ColorRGB BackgroundColor(Ray const &ray) const;

private:
	Image m_image;
};

