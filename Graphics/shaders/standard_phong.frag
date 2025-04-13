#version 460
#include "descriptors.glh"

layout(location = 0) in vec3 fPosition;
layout(location = 1) in vec3 fNormal;
layout(location = 2) in vec2 fTexCoord;

layout(location = 0) out vec4 FragColor;

/*
layout(set = 2, binding = 0) uniform MaterialParameters {
	uint albedoTexIdx;
	uint normalTexIdx;
	uint emissiveTexIdx;
} uMaterial;
*/

const vec3 lightDir = normalize(vec3(-1, -1, 1));

void main()
{
	// FragColor = vec4(uMaterial.albedoTexIdx, uMaterial.normalTexIdx, uMaterial.emissiveTexIdx, 1.0);
	// FragColor = vec4(texture(sampler2D(uGlobalTextures2D[0], uGlobalSamplers[0]), fTexCoord).rgb, 1.0);

	FragColor = vec4(fNormal, 1.0);
}

