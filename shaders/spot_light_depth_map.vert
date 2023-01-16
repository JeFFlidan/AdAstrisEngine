#version 450

#extension GL_GOOGLE_include_directive : require
#include "data.h"

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

layout(set = 1, binding = 0) uniform SpotLightData
{
	SpotLight spotLight;
};

void main()
{
	uint id = instanceBuffer.IDs[gl_InstanceIndex];
	gl_Position = spotLight.lightSpaceMat * objectBuffer.data[id].model * vec4(aPos, 1.0);
}
