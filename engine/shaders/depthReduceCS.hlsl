#include "common.hlsli"
PUSH_CONSTANT(depthReduceData, DepthReduceData);

[numthreads(DEPTH_REDUCE_GROUP_SIZE, DEPTH_REDUCE_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (all(DTid.xy < float2(depthReduceData.levelWidth, depthReduceData.levelHeight)))
	{
		SamplerState sampler = bindlessSamplers[SAMPLER_MINIMUM_NEAREST_CLAMP];
		float2 texCoord = (DTid.xy + 0.5f) / float2(depthReduceData.levelWidth, depthReduceData.levelHeight);
		float4 depths = bindlessTextures2D[NonUniformResourceIndex(depthReduceData.inDepthTextureIndex)].Gather(sampler, texCoord);
		bindlessRWTextures2DFloat[NonUniformResourceIndex(depthReduceData.outDepthTextureIndex)][DTid.xy] = max(max(depths.x, depths.y), max(depths.z, depths.w));
	}
}
