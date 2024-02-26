#include "common.hlsli"

#if defined(__spirv__)
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<CullingInstanceIndices> bindlessCullingInstanceIndices[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<DrawIndexedIndirectCommand> bindlessIndirectCommands[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] StructuredBuffer<CullingParams> bindlessCullingParams[];
#endif

PUSH_CONSTANT(indices, IndirectBufferIndices);

CullingInstanceIndices get_culling_instance_indices(uint index)
{
	return bindlessCullingInstanceIndices[indices.cullingInstanceIndicesBufferIndex][index];
}

bool is_visible(in uint objectID, in CullingParams cullingParams)
{
	RendererModelInstance modelInstance = bindlessStructuredModelInstances[get_frame().modelInstanceBufferIndex][objectID];
	RendererSphereBounds bounds = modelInstance.sphereBounds;
	bounds.center = mul(modelInstance.transform.get_matrix(), float4(bounds.center, 1.0f)).xyz;
	//center = mul(modelInstance.transform.get_matrix(), float4(center, 1.0f)).xyz;
	bounds.radius = bounds.radius * max(modelInstance.scale.x, max(modelInstance.scale.y, modelInstance.scale.z));
	
	bool visible = true;

	RendererFrustum frustum = get_camera(cullingParams.cameraIndex).frustum;
	
	[branch]
	if (cullingParams.isFrustumCullingEnabled)
	{
		visible = frustum.check(bounds);
	}

	return visible;
}

[numthreads(CULLING_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	CullingInstanceIndices instanceIndices = get_culling_instance_indices(DTid.x);
	CullingParams params = bindlessCullingParams[indices.cullingParamsBufferIndex][indices.cullingParamsIndex];

	[branch]
	if (DTid.x < params.drawCount)
	{
		[branch]
		if (is_visible(instanceIndices.objectID, params))
		{
			uint outputVal;
			InterlockedAdd(bindlessIndirectCommands[indices.indirectCommandsBufferIndex][instanceIndices.batchID].instanceCount, 1, outputVal);
			uint instanceIndex = bindlessIndirectCommands[indices.indirectCommandsBufferIndex][instanceIndices.batchID].firstInstance + outputVal;
			bindlessStructuredModelInstanceIDs[indices.rendererModelInstanceIdBufferIndex][instanceIndex].id = instanceIndices.objectID;
		}
	}
}
