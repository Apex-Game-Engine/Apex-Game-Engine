#version 460
#include "descriptors.glh"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;

layout (push_constant) uniform PushConstants { uint modelId; };

void main()
{
	mat4 V  = uCamera[0].view;
	mat4 P  = uCamera[0].projection;
	mat4 M  = uTransforms[0].models[modelId];
	mat4 MV = V * M;
	mat4 VP = P * V;

	vec4 worldPos    = M * vec4(inPosition, 1.0);
	vec4 viewPos     = VP * worldPos;

	vec4 worldNormal = MV * vec4(inNormal, 0.0);

	vPosition   = worldPos.xyz;
	vNormal	    = worldNormal.xyz;
	vTexCoord   = inTexCoord;

	gl_Position = viewPos;
}
