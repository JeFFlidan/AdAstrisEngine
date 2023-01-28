#version 450

#extension GL_GOOGLE_include_directive : require
#include <shaders/common_includes/data_structs/data.h>

#define MAX_FRAGMENT_COUNT 128

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 3) buffer LinkedListBuffer
{
	Node nodes[];
};

layout(set = 0, binding = 4, r32ui) uniform uimage2D headIndexImage;
layout(set = 0, binding = 5) uniform sampler2D opaqueColorAttach;

void main()
{
	Node fragments[MAX_FRAGMENT_COUNT];
	int count = 0;

	uint nodeIdx = imageLoad(headIndexImage, ivec2(gl_FragCoord.xy)).r;

	while (nodeIdx != 0xffffffff && count < MAX_FRAGMENT_COUNT)
	{
		fragments[count] = nodes[nodeIdx];
		nodeIdx = fragments[count].next;
		++count;
	}

	for (uint i = 1; i < count; ++i)
	{
		Node insert = fragments[i];
		uint j = i;
		while (j > 0 && insert.depth > fragments[j - 1].depth)
		{
			fragments[j] = fragments[j - 1];
			--j;
		}

		fragments[j] = insert;
	}

	vec4 color = texelFetch(opaqueColorAttach, ivec2(gl_FragCoord.xy), 0);
	for (int i = 0; i < count; ++i)
	{
		color = mix(color, fragments[i].color, fragments[i].color.a);
	}

	outColor = color;
}
