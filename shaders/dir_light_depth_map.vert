#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

struct ObjectData
{
	mat4 model;
	vec4 radius;
	vec4 extents;
};

struct DirectionLight
{
	vec4 direction;
	vec4 colorAndIntensity;	// w = intensity
	mat4 lightViewMat;
	mat4 lightProjMat;
};

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
	mat4 lightSpaceMatrix = dirLight.lightProjMat * dirLight.lightViewMat;
	gl_Position = lightSpaceMatrix * objectBuffer.data[id].model * vec4(aPos, 1.0);
}

