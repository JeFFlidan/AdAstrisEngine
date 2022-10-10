#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vColor;
layout(location = 3) in vec2 vUv;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform CameraBuffer{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraData;

struct ObjectData
{
	vec4 color;
	mat4 model;
};

layout(std430, set = 1, binding = 0) buffer ObjectBuffer
{
	ObjectData color[];
} objectBuffer;

layout(push_constant) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} PushConstants;

void main()
{
	mat4 modelMatrix = PushConstants.render_matrix;
	mat4 transformMatrix = cameraData.viewproj * modelMatrix;
	gl_Position = transformMatrix * vec4(vPosition, 1.f);
	outColor = objectBuffer.color[gl_BaseInstance].color;
}