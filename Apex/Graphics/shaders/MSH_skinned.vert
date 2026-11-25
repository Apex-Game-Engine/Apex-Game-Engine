#version 460
#include "common_include.glh"

layout(location = 0) in vec3	inPosition;
layout(location = 1) in vec2	inTexCoord;
layout(location = 2) in vec3	inNormal;
layout(location = 3) in vec3	inTangent;
layout(location = 4) in vec3	inBitangent;
layout(location = 5) in uvec4	inBoneIndices;
layout(location = 6) in vec4	inBoneWeights;

layout(location = 0) out vec3	outPosition;
layout(location = 1) out vec2	outTexCoord;
layout(location = 2) out mat3	outTBN;

DECLARE_UNIFORM(Transform, {
	mat4 model;
});

void main()
{
	mat4 V  = GetCamera().view;
	mat4 P  = GetCamera().projection;
	mat4 M  = RESOURCE(Transform, 2).model;
	mat4 MV = V * M;
	mat4 VP = P * V;

	vec4 worldPos    = M * vec4(inPosition, 1.0);
	vec4 viewPos     = VP * worldPos;

	vec3 N = normalize(vec3(M * vec4(inNormal, 0.0)));
	vec3 T = normalize(vec3(M * vec4(inTangent, 0.0)));
	vec3 B = normalize(vec3(M * vec4(inBitangent, 0.0)));

	outPosition = worldPos.xyz;
	outTexCoord = vec2(inTexCoord.x, 1.0 - inTexCoord.y);
	outTBN      = mat3(T, B, N);

	gl_Position = viewPos;
}
