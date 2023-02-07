#version 460

#extension GL_GOOGLE_include_directive : require
#include "data.h"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out uint id;
layout(location = 2) out vec4 curPos;
layout(location = 3) out vec4 oldPos;
layout(location = 4) out vec3 outNormal;
layout(location = 5) out vec3 outTangent;

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

layout(set = 0, binding = 3) uniform SettingsBuffer
{
	Settings settings;
};

layout(set = 0, binding = 4) uniform TAADataBuffer
{
	TAAData taaData;
};

void main()
{
	id = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[id].model;

	vec4 pos = cameraBuffer.viewProj * model * vec4(aPos, 1.0);
	
	if (settings.isTaaEnabled == 1)
	{
		float deltaWidth = 1.0 / settings.viewportRes.x;
		float deltaHeight = 1.0 / settings.viewportRes.y;
		uint framesCount = settings.totalFrames;
		uint index;
		if (framesCount < taaData.numSamples)
		{
			index = framesCount;
		}
		if (framesCount >= taaData.numSamples)
		{
			index = framesCount % taaData.numSamples;
		}
		
		vec2 jitter = vec2(taaData.haltonSequence[index].x, taaData.haltonSequence[index].y);
		jitter.x = ((jitter.x - 0.5f) / settings.viewportRes.x) * 2.0;
		jitter.y = ((jitter.y - 0.5f) / settings.viewportRes.y) * 2.0;
		pos += vec4(jitter * pos.w, 0.0, 0.0);
	}
	
	gl_Position = pos;

	outTexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	outNormal = normalMatrix * normalize(aNormal);
	outTangent = normalMatrix * normalize(aTangent);

	curPos = cameraBuffer.viewProj * model * vec4(aPos, 1.0);
	oldPos = cameraBuffer.proj * cameraBuffer.oldView * model * vec4(aPos, 1.0);
}
