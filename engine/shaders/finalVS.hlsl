struct VertexInput
{
	uint vertexID : SV_VertexID;
};

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoords : TEXCOORD1;
};

PixelInput main(VertexInput input)
{
	float3 positions[6] = {
		 float3(-1.0f, 1.0f, 0.0f),
		 float3(-1.0f, -1.0f,0.0f),
		 float3(1.0f, -1.0f, 0.0f),
		 float3(1.0f, -1.0f, 0.0f),
		 float3( 1.0f, 1.0f, 0.0f),
		 float3(-1.0f, 1.0f, 0.0f)
	};
	float2 texCoords[6] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	PixelInput output;
	output.position = float4(positions[input.vertexID], 1.0f);
	output.texCoords = texCoords[input.vertexID];
	return output;
}
