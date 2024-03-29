#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D texQuad;

void main()
{
	outColor = vec4(fragUV, 0.0, 1.0);
}
