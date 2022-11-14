#version 450

layout(local_size_x = 256) in;

layout(set = 0, binding = 5) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
};

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
}

layout(push_constant) uniform constants
{
	DrawCullData cullData;
};

layout(set = 0, binding = 4) uniform sampler2D depthPyramid;

struct ObjectData
{
	mat4 model;
	vec4 sphereBounds;
	vec4 extents;
};

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
};

layout(set = 0, binding = 3) buffer InstanceBuffer3
{
	uint IDs[];
} finalInstanceBuffer;



