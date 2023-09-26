#define CORE_SHADER_BASE_MODEL_LAYOUT
#include "coreHF.hlsli"

PixelInput main(VertexInput input)
{
	PixelInput output;
	
	float4 newLocation = mul(get_model_instance(input.instanceID).transform.get_matrix(), float4(input.location, 1.0f));
	output.location = mul(get_camera().viewProjection, newLocation);
	output.texCoord = input.texCoord;
	output.normal = input.normal;
	output.tangent = input.tangent;

	return output;
}
