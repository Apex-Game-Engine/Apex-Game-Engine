#version 450
#extension GL_KHR_vulkan_glsl : enable

// Uniforms
layout (binding = 0) uniform CameraUniforms
{
	//vec4 Position;
	mat4 model; // REMOVE FROM HERE
	mat4 view;
	mat4 projection;
	//mat4 ViewProjection;
	//mat4 InverseView;
} ubo;

// Inputs
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;

// Outputs
layout (location = 0) out vec4 v_color;

void main()
{
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
	v_color = in_color;
}
