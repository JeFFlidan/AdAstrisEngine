struct VertexInput
{
	uint vertexID : SV_VertexID;
};

struct PixelInput
{
	float4 position : SV_Position;
};

PixelInput main(VertexInput input)
{
	float3 positions[3] = {
		float3(1.f,1.f, 0.0f),
		float3(-1.f,1.f, 0.0f),
		float3(0.f,-1.f, 0.0f)
	};

	PixelInput output;
	output.position = float4(positions[input.vertexID], 1.0f);
	return output;
}
