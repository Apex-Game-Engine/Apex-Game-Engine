#version 450
#extension GL_KHR_vulkan_glsl : enable

// Uniforms
layout (binding = 0) uniform CameraUniforms
{
	//vec4 Position;
	mat4 view;
	mat4 projection;
	//mat4 ViewProjection;
	//mat4 InverseView;
} ubo;

layout (binding = 1) uniform ModelUniforms
{
	mat4 model[110];
} mbo;

// Push Constants
layout (push_constant) uniform PushConstants
{
	//vec4 Position;
	//mat4 model;
	uint modelIndex;
} pc;

// Inputs
#if 1
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
#else
layout (location = 0) in vec3 in_position;
layout (location = 1) in float in_jointIndex;
layout (location = 2) in vec4 in_color;
#endif

// Outputs
layout (location = 0) out vec4 v_color;
layout (location = 1) out vec3 v_position;

void main()
{
	vec4 worldPos = mbo.model[pc.modelIndex + gl_InstanceIndex] * vec4(in_position, 1.0);

	gl_Position = ubo.projection * ubo.view * worldPos;
	v_color = in_color;
	v_position = worldPos.xyz;
}
