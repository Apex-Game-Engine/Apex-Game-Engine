#include <iostream>
#include <fstream>

#include "Camera.h"
#include "Image.h"
#include "Material.h"
#include "Raytracing.h"

#define SET_GLOBAL_CONFIG_ULTRA() \
	constexpr float32 ASPECT_RATIO = 16.f / 9.f; \
	constexpr uint32 IMAGE_WIDTH = 1920; \
	constexpr uint32 IMAGE_HEIGHT = IMAGE_WIDTH / ASPECT_RATIO; \
	constexpr int32 SAMPLES_PER_PIXEL = 4000; \
	constexpr int32 MAX_BOUNCES = 50

#define SET_GLOBAL_CONFIG_HIGH() \
	constexpr float32 ASPECT_RATIO = 16.f / 9.f; \
	constexpr uint32 IMAGE_WIDTH = 800; \
	constexpr uint32 IMAGE_HEIGHT = IMAGE_WIDTH / ASPECT_RATIO; \
	constexpr int32 SAMPLES_PER_PIXEL = 1000; \
	constexpr int32 MAX_BOUNCES = 50

#define SET_GLOBAL_CONFIG_MED() \
	constexpr float32 ASPECT_RATIO = 16.f / 9.f; \
	constexpr uint32 IMAGE_WIDTH = 640; \
	constexpr uint32 IMAGE_HEIGHT = IMAGE_WIDTH / ASPECT_RATIO; \
	constexpr int32 SAMPLES_PER_PIXEL = 400; \
	constexpr int32 MAX_BOUNCES = 50

#define SET_GLOBAL_CONFIG_LOW() \
	constexpr float32 ASPECT_RATIO = 16.f / 9.f; \
	constexpr uint32 IMAGE_WIDTH = 400; \
	constexpr uint32 IMAGE_HEIGHT = IMAGE_WIDTH / ASPECT_RATIO; \
	constexpr int32 SAMPLES_PER_PIXEL = 200; \
	constexpr int32 MAX_BOUNCES = 50

int main(int argc, char *argv[])
{
	logging::Logger::initialize();
	math::Random::init();

	SET_GLOBAL_CONFIG_HIGH();

	CameraParams cameraParams{};
	cameraParams.origin = Point3D{ 0.f, 0.f, 0.f };
	cameraParams.focal_length = 1.f;
	cameraParams.viewport_height = 2.0;
	cameraParams.viewport_width = ASPECT_RATIO * cameraParams.viewport_height;

	Camera camera(cameraParams);

	RayTraceableShapeList scene{};

	// Geometries
	
	{
		Sphere sphere{};
		sphere.center = { 0.1f, 0.15f, -1.f };
		sphere.radius = 0.25f;
		sphere.pMaterial = std::make_shared<materials::Glass>();
		scene.add(std::make_shared<Sphere>(sphere));
	}

	{
		Sphere sphere{};
		sphere.center = { 0.65f, 0.f, -2.f };
		sphere.radius = 0.5f;
		sphere.pMaterial = std::make_shared<materials::Lambertian>( ColorRGB{ 0.7f, 0.7f, 0.7f } );
		scene.add(std::make_shared<Sphere>(sphere));
	}

	{
		Sphere sphere{};
		sphere.center = { -0.65f, 0.f, -2.f };
		sphere.radius = 0.5f;
		sphere.pMaterial = std::make_shared<materials::Metal>( ColorRGB{ 0.8f, 0.75f, 0.3f } );
		scene.add(std::make_shared<Sphere>(sphere));
	}

	{
		XZRect rect;
		rect.minXZ = { -1.05f, -2.5f };
		rect.maxXZ = { 1.05f, -0.5f };
		rect.constY = 1.31f;
		rect.pMaterial = std::make_shared<materials::Lambertian>( 0.8f * ColorRGB{ 1.f, 1.f, 1.f } );
		scene.add(std::make_shared<XZRect>(rect));
	}

	{
		XZRect rect;
		rect.minXZ = { -1.f, -1.5f };
		rect.maxXZ = { -0.5f, -0.5f };
		rect.constY = 1.3f;
		rect.pMaterial = std::make_shared<materials::Emissive>( ColorRGB{ 5.f, 5.f, 5.f } );
		scene.add(std::make_shared<XZRect>(rect));
	}

	{
		YZRect rect;
		rect.minYZ = { -0.51f, -2.5f };
		rect.maxYZ = { 1.3f, -0.5f };
		rect.constX = 1.0f;
		rect.pMaterial = std::make_shared<materials::Lambertian>( ColorRGB{ 1.f, 0.2f, 0.1f } );
		scene.add(std::make_shared<YZRect>(rect));
	}

	{
		YZRect rect;
		rect.minYZ = { -0.51f, -2.5f };
		rect.maxYZ = { 1.3f, -0.5f };
		rect.constX = -1.0f;
		rect.pMaterial = std::make_shared<materials::Lambertian>( ColorRGB{ 0.1f, 0.2f, 1.0f } );
		scene.add(std::make_shared<YZRect>(rect));
	}

	{
		XYRect rect;
		rect.minXY = { -1.f, -0.51f };
		rect.maxXY = { 1.f, 1.3f };
		rect.constZ = -2.5f;
		rect.pMaterial = std::make_shared<materials::Lambertian>( ColorRGB{ 1.f, 1.f, 1.f } );
		scene.add(std::make_shared<XYRect>(rect));
	}

	{
		//Box box({ 0.f, 1.f, -1.5f }, 0.25f);
		//box.pMaterial = std::make_shared<materials::Emissive>( ColorRGB{ 5.f, 3.f, 1.f } );
		//scene.add(std::make_shared<Box>(box));
	}

	{
		Sphere sphere{};
		sphere.center = { 0.f, -100.5f, -1.f };
		sphere.radius = 100.f;
		sphere.pMaterial = std::make_shared<materials::Lambertian>( ColorRGB{ 0.8f, 1.f, 0.7f } );
		scene.add(std::make_shared<Sphere>(sphere));
	}


	RayTracer raytracer{ IMAGE_WIDTH, IMAGE_HEIGHT };
	//raytracer.mode = RayTracer::eDebugRenderNormals;
	raytracer.TraceRays(camera, scene, SAMPLES_PER_PIXEL, MAX_BOUNCES);

	std::ofstream outfile;

	outfile.open("raytracer_output.ppm");

	if (outfile.is_open())
		WriteImagePPM(outfile, raytracer.GetImage());

	outfile.close();

	return 0;
}
