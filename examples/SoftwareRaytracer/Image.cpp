#include "Image.h"

ColorRGB const& Image::get(uint32 u, uint32 v) const
{
	return pixels[v*width + u];
}

void Image::set(uint32 u, uint32 v, ColorRGB const& color) const
{
	pixels[v*width + u] = color;
}

void Image::normalizeImageSamples(int32 samples_per_pixel)
{
	for (int32 j = 0; j < height; j++)
	{
		for (int32 i = 0; i < width; i++)
		{
			ColorRGB& pixelColor = pixels[j * width + i];
			pixelColor /= static_cast<float32>(samples_per_pixel);
		}
	}
}

void WriteColorPPM(std::ostream &out, ColorRGB const &color)
{
	out << static_cast<uint32>(256 * clamp(color.x, 0.f, 0.999f)) << ' '
		<< static_cast<uint32>(256 * clamp(color.y, 0.f, 0.999f)) << ' '
		<< static_cast<uint32>(256 * clamp(color.z, 0.f, 0.999f)) << '\n';
}

void WriteImagePPM(std::ostream &out, Image const &image)
{
	out << "P3\n" << image.width << ' ' << image.height << "\n255\n";
	for (uint32 j = 0; j < image.height; ++j)
	{
		for (uint32 i = 0; i < image.width; ++i)
		{
			ColorRGB const &pixelColor = image.get(i, image.height - j - 1);
			WriteColorPPM(out, pixelColor);
		}
	}
}
