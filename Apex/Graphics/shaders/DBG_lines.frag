#version 460
#include "common_include.glh"

layout(location = 0) out vec4 FragColor;

layout (push_constant) uniform PushConstants {
	vec4 color;
	uint bufferIndex;
};

void main()
{
	FragColor = vec4(1.0);
}