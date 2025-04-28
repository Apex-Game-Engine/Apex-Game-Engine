#version 460
#include "descriptors.glh"

layout(set = BindlessStorageBufferSet, binding = 0) buffer readonly Positions { vec4 positions[]; } uPositions[];

layout (push_constant) uniform PushConstants {
	vec4 color;
	uint bufferIndex;
};

void main()
{
	mat4 V = uCamera[0].view;
	mat4 P = uCamera[0].projection;
	mat4 VP = P * V;
	vec4 pos = vec4(uPositions[bufferIndex].positions[gl_VertexIndex].xyz, 1.0);
	vec4 viewPos = VP * pos;

	gl_Position = viewPos;
}
