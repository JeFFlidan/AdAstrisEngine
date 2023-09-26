//#include "coreHF.hlsli"

struct PixInput
{
	float4 position : SV_Position;
	float4 color : COLOR0;
};

float4 main(PixInput input) : SV_TARGET
{
	return input.color;
}