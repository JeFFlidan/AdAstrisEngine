#include "coreHF.hlsli"

struct VertInput
{
	uint vertexID : SV_VertexID;
};

struct PixInput
{
	float4 position : SV_Position;
	float4 color : COLOR0;
};

PixInput main(VertInput input)
{
	float3 positions[3] = {
		float3(1.f,1.f, 0.0f),
		float3(-1.f,1.f, 0.0f),
		float3(0.f,-1.f, 0.0f)
	};

	PixInput output;
	output.position = float4(positions[input.vertexID], 1.0f);
	output.color = mul(get_camera().projection, float4(0.0f, 1.0f, 1.0f, 1.0f));
	output.color.x *= get_frame().modelInstanceBufferIndex;
	return output;
}
