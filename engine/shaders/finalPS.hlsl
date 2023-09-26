struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoords : TEXCOORD1;
};

[[vk::binding(0, 0)]] Texture2D<float4> inputText;
[[vk::binding(1, 0)]] SamplerState sampl;

float4 main(PixelInput input) : SV_TARGET
{
	return inputText.Sample(sampl, input.texCoords);
}
