#version 450

#extension GL_GOOGLE_include_directive : require

#include <shaders/common_includes/data_structs/data.h>

layout(local_size_x = 256) in;

layout(set = 0, binding = 5) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
} cameraBuffer;

struct DrawCullData
{
	mat4 view;
	float P00, P11, znear, zfar; // symmetric projection parameters
	float frustum[4];  // data for left/right/top/bottom frustum planes
	float lodBase, lodStep;  // lod distance i = base * pow(step, i)
	float pyramidWidth, pyramidHeight;  // depth pyramid size in texels

	uint drawCount;

	int cullingEnabled;
	int lodEnabled;
	int occlusionEnabled;
	int distCull;
	int AABBcheck;
	float aabbmin_x;
	float aabbmin_y;
	float aabbmin_z;
	float aabbmax_x;
	float aabbmax_y;
	float aabbmax_z;	
};

layout(push_constant) uniform constants
{
	DrawCullData cullData;
};

layout(set = 0, binding = 4) uniform sampler2D depthPyramid;

/*struct ObjectData
{
	mat4 model;
	vec4 sphereBounds;
	vec4 extents;
};*/

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData objects[];
} objectBuffer;

struct DrawCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;
	uint objectID;
	uint batchID;
};

layout(set = 0, binding = 1) buffer InstanceBuffer
{
	DrawCommand Draws[];
} drawBuffer;

struct GPUInstance
{
	uint objectID;
	uint batchID;
};

layout(set = 0, binding = 2) readonly buffer InstanceBuffer2
{
	GPUInstance Instances[];
} compactInstanceBuffer;

layout(set = 0, binding = 3) buffer InstanceBuffer3
{
	uint IDs[];
} finalInstanceBuffer;

bool projectSphere(vec3 C, float r, float znear, float P00, float P11, out vec4 aabb)
{
	if (C.z < r + znear)
	{
		return false;
	}

	vec2 cx = -C.xz;
	vec2 vx = vec2(sqrt(dot(cx, cx) - r * r), r);
	vec2 minx = mat2(vx.x, vx.y, -vx.y, vx.x) * cx;
	vec2 maxx = mat2(vx.x, -vx.y, vx.y, vx.x) * cx;

	vec2 cy = -C.yz;
	vec2 vy = vec2(sqrt(dot(cy, cy) - r * r), r);
	vec2 miny = mat2(vy.x, vy.y, -vy.y, vy.x) * cy;
	vec2 maxy = mat2(vy.x, -vy.y, vy.y, vy.x) * cy;

	aabb = vec4(minx.x / minx.y * P00, miny.x / miny.y * P11, maxx.x / maxx.y * P00, maxy.x / maxy.y * P11);
	aabb = aabb.xwzy * vec4(0.5, -0.5, 0.5, -0.5) + vec4(0.5);	// clip space to uv space

	return true;
}

bool isVisible(uint objectIndex)
{
	uint index = objectIndex;

	vec4 sphereBounds = objectBuffer.objects[index].sphereBounds;
	mat4 model = objectBuffer.objects[index].model;

	vec3 center = sphereBounds.xyz;
	center = (cullData.view * model * vec4(center, 1.0)).xyz;

	float radius = sphereBounds.w;

	bool visible = true;

	// the left/top/right/bottom plane culling utilizes frustum symmetry to cull against two planes at the same time
	visible = visible && center.z * cullData.frustum[1] - abs(center.x) * cullData.frustum[0] > -radius;
	visible = visible && center.z * cullData.frustum[3] - abs(center.y) * cullData.frustum[2] > -radius;

	if (cullData.distCull != 0)
	{// the near/far plane culling uses camera space Z directly
		visible = visible && center.z + radius > cullData.znear && center.z - radius < cullData.zfar;
	}

	visible = visible || cullData.cullingEnabled == 0;

	//flip Y because we access depth texture that way
	center.y *= -1;

	if (visible && cullData.occlusionEnabled != 0)
	{
		vec4 aabb;
		if (projectSphere(center, radius, cullData.znear, cullData.P00, cullData.P11, aabb))
		{
			float width = (aabb.z - aabb.x) * cullData.pyramidWidth;
			float height = (aabb.w - aabb.y) * cullData.pyramidHeight;

			float level = floor(log2(max(width, height)));

			// Sampler is set up to do min reduction, so this computes the minimum depth of 2x2 texel quad

			float depth = textureLod(depthPyramid, (aabb.xy + aabb.zw) * 0.5, level).x;
			float depthSphere = cullData.znear / (center.z - radius);

			visible = visible && depthSphere >= depth;
		}
	}

	return visible;
}

bool isVisibleAABB(uint objectIndex)
{
	uint index = objectIndex;

	vec4 sphereBounds = objectBuffer.objects[index].sphereBounds;

	vec3 center = sphereBounds.xyz;
	float radius = sphereBounds.w;

	bool visible = true;

	vec3 aabbmin = vec3(cullData.aabbmin_x, cullData.aabbmin_y, cullData.aabbmin_z) + vec3(radius);
	vec3 aabbmax = vec3(cullData.aabbmax_x, cullData.aabbmax_y, cullData.aabbmax_z) + vec3(radius);

	visible = visible && (center.x > aabbmin.x) && (center.x < aabbmax.x);
	visible = visible && (center.y > aabbmin.y) && (center.y < aabbmax.y);
	visible = visible && (center.z > aabbmin.z) && (center.z < aabbmax.z);

	return visible;
}

void main()
{
	uint gID = gl_GlobalInvocationID.x;
	if (gID < cullData.drawCount)
	{
		uint objectID = compactInstanceBuffer.Instances[gID].objectID;
		bool visible = false;
		if (cullData.AABBcheck == 0)
		{
			visible = isVisible(objectID);			
		}
		else
		{
			visible = isVisibleAABB(objectID);
		}

		if (visible)
		{
			uint batchIndex = compactInstanceBuffer.Instances[gID].batchID;
			uint countIndex = atomicAdd(drawBuffer.Draws[batchIndex].instanceCount, 1);

			uint instanceIndex = drawBuffer.Draws[batchIndex].firstInstance + countIndex;

			finalInstanceBuffer.IDs[instanceIndex] = objectID;
		}
	}
}

