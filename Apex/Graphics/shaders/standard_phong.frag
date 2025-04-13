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

const vec3 lights[2] = vec3[2](
	normalize(vec3(-1, -1, 1)),
	normalize(vec3(1, 1, 1))
);

const vec3 lightColors[2] = vec3[2](
	vec3(0.85, 0.15, 0.12),
	vec3(0.08, 0.25, 0.78)
);

void main()
{
	// FragColor = vec4(uMaterial.albedoTexIdx, uMaterial.normalTexIdx, uMaterial.emissiveTexIdx, 1.0);
	// FragColor = vec4(texture(sampler2D(uGlobalTextures2D[0], uGlobalSamplers[0]), fTexCoord).rgb, 1.0);

	vec3 color = vec3(0.15, 0.08, 0.02);

	vec3 N = normalize(fNormal);

	for (uint i = 0; i < 2; i++)
	{
		vec3 L = lights[i];
		color += vec3(dot(N, L)) * lightColors[i];
	}

	FragColor = vec4(color, 1.0);
}

