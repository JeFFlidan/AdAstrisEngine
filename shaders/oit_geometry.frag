#version 450

layout(early_fragment_tests) in;

#extension GL_GOOGLE_include_directive : require
#include "data.h"

/*struct Node
{
	vec4 color;
	float depth;
	uint next;
};*/

layout(set = 0, binding = 3) buffer coherent LinkedListBuffer
{
	Node nodes[];
};

layout(set = 0, binding = 4) buffer GeometryBuffer
{
	uint count;
	uint maxNodeCount;
};

layout(set = 0, binding = 5, r32ui) uniform coherent uimage2D headIndexImage;

layout(push_constant) uniform PushConsts
{
	vec4 color;
} pushConsts;

void main()
{
	uint nodeIdx = atomicAdd(count, 1);

	if (nodeIdx < maxNodeCount)
	{
		uint prevHeadIndex = imageAtomicExchange(headIndexImage, ivec2(gl_FragCoord.xy), nodeIdx);

		nodes[nodeIdx].color = pushConsts.color;
		nodes[nodeIdx].depth = gl_FragCoord.z;
		nodes[nodeIdx].next = prevHeadIndex;
	}
}
