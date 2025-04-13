#version 460
#include "descriptors.glh"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;

void main()
{
	vec4 worldPos = uCamera[0].projection * uCamera[0].view * vec4(inPosition, 1.0);
	vPosition = worldPos.xyz;
	vTexCoord = inTexCoord;
	vNormal = inNormal;
	gl_Position = worldPos;
}
