#version 450

#include <shaders/common/data.h>

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(set = 0, binding = 0) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 1) buffer readonly InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

layout(set = 1, binding = 0) uniform DirLightData
{
	DirectionLight dirLight;
};

void main()
{
	uint id = instanceBuffer.IDs[gl_InstanceIndex];
	gl_Position = dirLight.lightSpaceMat * objectBuffer.data[id].model * vec4(aPos, 1.0);
}
