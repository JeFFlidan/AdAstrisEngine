#version 460

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out uint id;

layout(set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraBuffer;

struct ObjectData
{
	mat4 model;
	vec4 radius;
	vec4 extents;
};

layout(set = 1, binding = 0) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 1, binding = 1) buffer readonly InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

void main()
{
	id = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[id].model;
	mat4 transformMatrix = cameraBuffer.viewproj * model;
	gl_Position = transformMatrix * vec4(aPos, 1.f);
	texCoords = aTexCoords;
}
