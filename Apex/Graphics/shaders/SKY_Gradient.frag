#version 460

layout (location = 0) in vec2 fUV;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(fUV, 0.0, 1.0);
}