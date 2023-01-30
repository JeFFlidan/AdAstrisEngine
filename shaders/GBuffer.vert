#version 460

#extension GL_GOOGLE_include_directive : require
#include "data.h"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out uint id;
layout(location = 2) out mat3 TBN;

layout(set = 0, binding = 0) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 1) buffer readonly InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

layout(set = 0, binding = 2) uniform CameraBuffer
{
	CameraData cameraBuffer;
};

void main()
{
	id = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[id].model;
	gl_Position = cameraBuffer.viewProj * model * vec4(aPos, 1.0);

	outTexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 N = normalize(normalMatrix * aNormal);
	vec3 T = normalize(normalMatrix * aTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
}

