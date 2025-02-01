#version 460

struct Camera {
	mat4 view;
	mat4 projection;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 vPosition;

layout(set = 0, binding = 0) uniform FrameData {
	Camera camera;
};

void main()
{
	vec4 worldPos = camera.projection * camera.view * vec4(inPosition, 1.0);
	vPosition = worldPos.xyz;
	gl_Position = worldPos;
}
