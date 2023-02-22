#version 450

#extension GL_GOOGLE_include_directive : require
#include <shaders/common_includes/data_structs/data.h>

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

layout(location = 0) out vec4 curPos;
layout(location = 1) out vec4 oldPos;

layout(set = 0, binding = 0) buffer readonly ObjectBuffer
{
	ObjectData data[];
} objectBuffer;

layout(set = 0, binding = 1) buffer readonly InstanceBuffer
{
	uint IDs[];
} instanceBuffer;

layout(set = 0, binding = 3) uniform CameraBuffer
{
	CameraData camera;
};

layout(set = 0, binding = 4) uniform SettingsBuffer
{
	Settings settings;
};

layout(set = 0, binding = 5) uniform TAADataBuffer
{
	TAAData taaData;
};

void main()
{
	uint ind = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 model = objectBuffer.data[ind].model;

	vec4 pos = camera.viewProj * model * vec4(aPos, 1.0);
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

	curPos = camera.viewProj * model * vec4(aPos, 1.0);
	oldPos = camera.proj * camera.oldView * model * vec4(aPos, 1.0);
}
