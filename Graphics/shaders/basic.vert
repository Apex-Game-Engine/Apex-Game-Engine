#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 vertColor;

void main()
{
	gl_Position = vec4(inPosition, 1.0);
	vertColor = inColor;
}
