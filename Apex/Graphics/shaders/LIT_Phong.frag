#version 460
#include "common_include.glh"

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 FragColor;

struct DirLight {
	vec4 direction;
	vec4 color;
};

struct PointLight {
	vec4 position;
	vec4 color;
};

const DirLight dirLights[] = {
	DirLight( normalize(vec4(-1, -1, -1, 0)), vec4(0.15, 0.18, 0.12, 1.0) )
};

const PointLight pointLights[] = {
	PointLight( vec4(-1, 5, -5, 0), vec4(0.8, 0.8, 0.8, 1.0) )
};

layout(push_constant) uniform PushConstants {
	uint albedo;
	uint normal;
	uint emissive;
	uint depth;
	uint useDynamicLights;
} PC;

struct LightParams {
	vec4 pdi; // Position | Direction, Intensity
	uint tc; // Type, Color
};

DECLARE_BUFFER(std430, readonly, Lights, {
	uint count;
	LightParams lights[];
});

#define USE_STATIC_LIGHTS 0
#define LIGHT_BUFFER_INDEX 1

#define LightType_Point			0
#define LightType_Directional	1
#define LightType_Spot			2

uint GetLightCount()
{
	return RESOURCE(Lights, LIGHT_BUFFER_INDEX).count;
}

void GetLightParamsNoCheck(uint index, out uint type, out vec3 color, out vec3 pd, out float intensity) {
	LightParams lp = RESOURCE(Lights, LIGHT_BUFFER_INDEX).lights[index];
	uint r = lp.tc & 0xFF;
	uint g = (lp.tc >> 8) & 0xFF;
	uint b = (lp.tc >> 16) & 0xFF;
	type = (lp.tc >> 24) & 0xFF;
	color = vec3(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0);
	pd = lp.pdi.xyz;
	intensity = lp.pdi.w;
}

uint GetLightParams(uint index, out uint type, out vec3 color, out vec3 pd, out float intensity) {
	if (index >= RESOURCE(Lights, LIGHT_BUFFER_INDEX).count)
		return 0;
	GetLightParamsNoCheck(index, type, color, pd, intensity);
	return 1;
}

float Filter(float intensity) {
	//return (intensity > 0.3) ? 1.0 : 0.0;
	return intensity;
}

float CalcPhongDiffuseIntensity(in vec3 L, in vec3 N) {
	return max(dot(N, L), 0.0);
}

float CalcBlinnPhongSpecularIntensity(in vec3 L, in vec3 V, in vec3 N) {
	vec3 H = normalize(L + V);
	return pow(max(dot(N, H), 0.0), 16.0);
}

vec3 CalcViewPos(in vec2 uv, in float depth) {
	vec3 clipPos = vec3(uv, depth) * 2.0 - 1.0;
	mat4 invP = GetCamera().inverseProjection;
	vec4 viewPos = vec4(vec2(invP[0][0], invP[1][1]) * clipPos.xy, -1.0, invP[2][3] * clipPos.z + invP[3][3]);
	return vec3(viewPos.xyz / viewPos.w);
}

vec3 CalcWorldPos(in vec3 viewPos) {
	mat4 invV = GetCamera().inverseView;
	vec4 worldPos = invV * vec4(viewPos, 1.0);
	return worldPos.xyz;
}

vec3 SurfaceColor(vec2 uv) {
	vec3 color = vec3(0);
	vec3 Id = vec3(0.02);
	vec3 Is = vec3(0.0);

	vec4 albedo = texture(sampler2D(uGlobalTextures2D[PC.albedo], uGlobalSamplers[0]), uv).rgba;
	vec3 normal = texture(sampler2D(uGlobalTextures2D[PC.normal], uGlobalSamplers[0]), uv).rgb;
	vec3 emissive = texture(sampler2D(uGlobalTextures2D[PC.emissive], uGlobalSamplers[0]), uv).rgb;
	float depth = texture(sampler2D(uGlobalTextures2D[PC.depth], uGlobalSamplers[0]), uv).r;
	
	if (albedo.a == 0.0)
		return color;

	vec3 viewPos = CalcViewPos(uv, depth);
	vec3 worldPos = CalcWorldPos(viewPos);
	vec3 N = normalize(normal);
	vec3 V = normalize(-viewPos);

	if (USE_STATIC_LIGHTS == 0)
	{
		uint lightCount = GetLightCount();
		for (uint i = 0; i < lightCount; i++) {
			uint type; vec3 color; vec3 pd; float intensity;
			GetLightParamsNoCheck(i, type, color, pd, intensity);
			if (type == LightType_Point) {
				vec3 PL = pd.xyz - worldPos;
				float d = length(PL);
				vec3 L = normalize(PL);
				float Id1 = CalcPhongDiffuseIntensity(L, N);
				Id1 /= d * d;
				Id += Filter(Id1) * color.rgb;
				float Is1 = CalcBlinnPhongSpecularIntensity(L, V, N);
				Is += Filter(Is1) * color.rgb;
			}
			else if (type == LightType_Directional) {
				vec3 L = normalize(-pd.rgb);
				float Id1 = CalcPhongDiffuseIntensity(L, N);
				Id += Filter(Id1) * color.rgb;
				float Is1 = CalcBlinnPhongSpecularIntensity(L, V, N);
				Is += Filter(Is1) * color.rgb;
			}
		}
	}
	else
	{
		for (uint i = 0; i < 1; i++) {
			vec3 PL = pointLights[i].position.xyz - worldPos;
			float d = length(PL);
			vec3 L = normalize(PL);
			float Id1 = CalcPhongDiffuseIntensity(L, N);
			Id1 /= d * d;
			Id += Filter(Id1) * pointLights[i].color.rgb;
			float Is1 = CalcBlinnPhongSpecularIntensity(L, V, N);
			Is += Filter(Is1) * pointLights[i].color.rgb;
		}
		for (uint i = 0; i < 1; i++) {
			vec3 L = normalize(-dirLights[i].direction.rgb);
			float Id1 = CalcPhongDiffuseIntensity(L, N);
			Id += Filter(Id1) * dirLights[i].color.rgb;
			float Is1 = CalcBlinnPhongSpecularIntensity(L, V, N);
			Is += Filter(Is1) * dirLights[i].color.rgb;
		}
	}

	vec3 I = Id + Is;
	color = I * albedo.rgb;
	return color;
}

void main() {
	vec3 color = SurfaceColor(inUV);
	FragColor = vec4(color, 1.0);
}

