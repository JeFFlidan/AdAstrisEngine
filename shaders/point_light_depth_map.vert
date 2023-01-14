#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_multiview : require

#include "data.h"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 lightPosition;
layout(location = 2) out float farPlane;

struct ObjectData
{
	mat4 model;
	vec4 radius;
	vec4 extents;
};

layout(set = 0, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 1) readonly buffer InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

layout(set = 1, binding = 0) uniform PointLightData
{
	PointLight pointLight;
};

void main()
{
	mat4 lightSpaceMat = pointLight.lightSpaceMat[gl_ViewIndex];
	uint id = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[id].model;
	gl_Position = lightSpaceMat * model * vec4(aPos, 1.0);

	fragPos = vec3(model * vec4(aPos, 1.0));
	lightPosition = vec3(pointLight.positionAndAttRadius);
	farPlane = pointLight.farPlane;
}
