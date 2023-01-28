#version 460

#extension GL_GOOGLE_include_directive : require
#include <shaders/common_includes/data_structs/data.h>

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out uint id;
layout(location = 2) out vec3 viewPos;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out mat3 TBN;

layout(set = 0, binding = 3) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 cameraPosition;
} cameraBuffer;

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

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 N = normalize(normalMatrix * aNormal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(T, N);

	TBN = mat3(T, B, N);

	fragPos = vec3(model * vec4(aPos, 1.0f));
	viewPos = vec3(cameraBuffer.cameraPosition);

	/*vec4 test1 = dirLights.casters[0].colorAndIntensity;
	vec4 test2 = pointLights.casters[0].color;
	vec4 test3 = spotLights.casters[0].color;

	for (int i = 0; i != sceneData.dirLightsAmount; ++i)
	{
		dirLights.casters[i].direction = vec4(TBN * vec3(dirLights.casters[i].direction), 1.0);
	}

	for (int i = 0; i != sceneData.pointLightsAmount; ++i)
	{
		pointLights.casters[i].position = vec4(TBN * vec3(pointLights.casters[i].position), 1.0);
	}

	for (int i = 0; i != sceneData.spotLightsAmount; ++i)
	{
		spotLights.casters[i].position = vec4(TBN * vec3(spotLights.casters[i].position), 1.0);
	}*/
}
