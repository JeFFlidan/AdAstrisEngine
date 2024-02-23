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

[numthreads(CULLING_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	CullingInstanceIndices instanceIndices = get_culling_instance_indices(DTid.x);
	CullingParams params = bindlessCullingParams[indices.cullingParamsBufferIndex][indices.cullingParamsIndex];
	if (DTid.x < params.drawCount)
	{
		uint outputVal;
		InterlockedAdd(bindlessIndirectCommands[indices.indirectCommandsBufferIndex][instanceIndices.batchID].instanceCount, 1, outputVal);
		uint instanceIndex = bindlessIndirectCommands[indices.indirectCommandsBufferIndex][instanceIndices.batchID].firstInstance + instanceIndices.objectID;
		bindlessStructuredModelInstanceIDs[indices.rendererModelInstanceIdBufferIndex][instanceIndex].id = instanceIndices.objectID;
	}
}
