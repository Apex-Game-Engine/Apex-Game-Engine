#version 460
#include "common_include.glh"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in mat3 inTBN;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outEmissive;

layout(push_constant) uniform MaterialProperties {
	vec4 albedoValue;
	vec4 roughnessMetallicAoValue;
	vec4 emissiveValue;
	uint albedoTex;
	uint normalTex;
	uint roughnessMetallicAoTex;
	uint emissiveTex;
} MAT;

void main() {
	vec3 albedo = MAT.albedoValue.rgb;
	vec3 emissive = MAT.emissiveValue.rgb;
	vec3 normal = vec3(0.5, 0.5, 1.0);

	if (MAT.albedoTex != -1) {
		albedo = texture(sampler2D(uGlobalTextures2D[MAT.albedoTex], GetNearestSampler()), inTexCoord).rgb;
	}
	if (MAT.emissiveTex != -1) {
		emissive = texture(sampler2D(uGlobalTextures2D[MAT.emissiveTex], GetNearestSampler()), inTexCoord).rgb;
	}
	if (MAT.normalTex != -1) {
		normal = texture(sampler2D(uGlobalTextures2D[MAT.normalTex], GetNearestSampler()), inTexCoord).rgb;
	}

	normal = normal * 2.0 - 1.0;
	normal = normalize(inTBN * normal);

	outAlbedo = vec4(albedo, 1.0);
	outNormal = vec4(normal, 1.0);
	outEmissive = vec4(emissive, 1.0);
}
