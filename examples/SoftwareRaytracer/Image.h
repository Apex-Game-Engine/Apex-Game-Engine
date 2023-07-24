#pragma once
#include <ostream>

#include "Foundation/Types.h"
#include "Foundation/detail/Vector3.h"
#include "Foundation/detail/Color.h"

using namespace apex;
using namespace apex::math;

struct Image
{
	uint32 width, height;
	ColorRGB *pixels;

	ColorRGB const& get(uint32 u, uint32 v) const;
	void set(uint32 u, uint32 v, ColorRGB const& color) const;

	void normalizeImageSamples(int32 samples_per_pixel);
};

void WriteColorPPM(std::ostream &out, ColorRGB const &color);
void WriteImagePPM(std::ostream &out, Image const &image);
