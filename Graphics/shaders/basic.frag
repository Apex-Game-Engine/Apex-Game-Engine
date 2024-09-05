#version 450

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec3 v_position;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D texQuad;

const vec3 ambientColor = vec3(0.05, 0.03, 0.01);
const vec3 lightPos = vec3(0.0, 5.0, 2.0);

void main()
{
	vec3 fragToLight = normalize(lightPos - v_position);
	vec3 normal = -normalize(cross(dFdx(v_position), dFdy(v_position)));
	float diffuseFactor = max(dot(fragToLight, normal), 0.0);

	outColor = v_color * vec4(vec3(diffuseFactor) + ambientColor, 1);
}
