#define CORE_SHADER_BASE_MODEL_LAYOUT
#include "coreHF.hlsli"

float4 main(PixelInput input) : SV_TARGET
{
	return float4(0.5f, 0.2f, 0.4f, 1.0f);
}
