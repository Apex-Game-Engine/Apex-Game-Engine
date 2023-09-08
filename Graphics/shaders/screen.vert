#version 450
#extension GL_KHR_vulkan_glsl : enable

vec2 gPositions[3] = vec2[](
	vec2(-1.0, -3.0),
	vec2( 3.0,  1.0),
	vec2(-1.0,  1.0)
);

vec3 gColors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

vec2 gUVs[3] = vec2[](
	vec2(0.0, 2.0),
	vec2(2.0, 0.0),
	vec2(0.0, 0.0)
);

layout (location = 0) out vec3 vertColor;
layout (location = 1) out vec2 vertUV;

void main()
{
	gl_Position = vec4(gPositions[gl_VertexIndex], 0.0, 1.0);
	vertColor = gColors[gl_VertexIndex];
	vertUV = gUVs[gl_VertexIndex];
}
