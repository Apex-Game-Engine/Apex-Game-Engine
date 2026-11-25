#version 460
#include "common_include.glh"

DECLARE_BUFFER(std430, readonly, Positions, { vec4 positions[]; });

layout (push_constant) uniform PushConstants {
	vec4 color;
	uint bufferIndex;
};

void main()
{
	mat4 V = GetCamera().view;
	mat4 P = GetCamera().projection;
	mat4 VP = P * V;
	vec4 pos = vec4(RESOURCE(Positions, bufferIndex).positions[gl_VertexIndex].xyz, 1.0);
	vec4 viewPos = VP * pos;

	gl_Position = viewPos;
}
