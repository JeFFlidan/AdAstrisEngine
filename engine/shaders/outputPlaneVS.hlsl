#define CORE_SHADER_OUTPUT_PLANE
#include "coreHF.hlsli"

PixelInput main(VertexInput input)
{
	PixelInput output;
	
	output.location = float4(input.location, 1.0);
	output.texCoord = input.texCoord;
	return output;
}
