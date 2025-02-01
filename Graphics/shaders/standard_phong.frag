#version 460

layout(location = 0) in vec3 fPosition;

layout(location = 0) out vec4 FragColor;

// layout(set = 1, binding = 0) uniform Material {
// 	vec4 color;
// };

const vec3 lightDir = normalize(vec3(-1, -1, 1));

void main()
{
	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}

