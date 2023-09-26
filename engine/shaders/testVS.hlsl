cbuffer cbPerFrame : register(b0)
{
	float4x4 matVP;
	float4x4 matGeo;
};

struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float3 Tangent : TANGENT0;
};

struct VSOutput
{	
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 Texcoord : TEXCOORD0;
	float3 Tangent : TANGENT0;
	float3 Normal : NORMAL;
};

VSOutput main(VSInput vin)
{
	VSOutput vout = (VSOutput)0;

	vout.Position = mul(mul(float4(vin.Position, 1.0f), matGeo), matVP);
	vout.Color = float4(abs(vin.Normal), 1);
	vout.Texcoord = vin.Texcoord;
	vout.Tangent = vin.Tangent;
	vout.Normal = vin.Normal;

	return vout;
}
