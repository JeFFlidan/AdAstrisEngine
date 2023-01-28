#version 450

#extension GL_GOOGLE_include_directive : require
#include <shaders/common_includes/data_structs/data.h>

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(set = 0, binding = 0) uniform CameraBuffer
{
	CameraData camera;
};

layout(set = 0, binding = 1) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 2) buffer readonly InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

void main()
{
	uint ind = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[ind].model;
	gl_Position = camera.viewProj * model * vec4(aPos, 1.0);
}
