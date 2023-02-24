#version 450

#extension GL_EXT_nonuniform_qualifier : require
#include <shaders/common/data.h>

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat uint id;
layout(location = 2) in vec4 curPos;
layout(location = 3) in vec4 oldPos;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec3 tangent;

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gSurface;		// roughness and metallic
layout(location = 3) out vec2 gVelocity;

layout(set = 0, binding = 0) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 5) uniform sampler texSampler;

layout(set = 1, binding = 0) uniform texture2D baseColorTextures[];
layout(set = 2, binding = 0) uniform texture2D normalTextures[];
layout(set = 3, binding = 0) uniform texture2D armTextures[]; 

void main()
{
	uint baseColorTexId = objectBuffer.data[id].baseColorTexId;
	uint armTexId = objectBuffer.data[id].armTexId;
	uint normalTexId = objectBuffer.data[id].normalTexId;

	gAlbedo = texture(nonuniformEXT(sampler2D(baseColorTextures[baseColorTexId], texSampler)), texCoord);
	vec3 N = normalize(normal);
	vec3 T = normalize(tangent);
	vec3 B = cross(T, N);
	mat3 TBN = mat3(T, B, N);
	vec3 normal = texture(nonuniformEXT(sampler2D(normalTextures[normalTexId], texSampler)), texCoord).xyz;
	normal = normal * 2.0 - 1.0;
	normal = TBN * normalize(normal);
	gNormal = vec4(normal, 1.0);

	gSurface = vec4(texture(nonuniformEXT(sampler2D(armTextures[armTexId], texSampler)), texCoord).xyz, 1.0);
	
	vec2 newPos = (curPos.xy / curPos.w) * 0.5 + 0.5;
	vec2 prePos = (oldPos.xy / oldPos.w) * 0.5 + 0.5;
	
	gVelocity = (prePos - newPos);
}
