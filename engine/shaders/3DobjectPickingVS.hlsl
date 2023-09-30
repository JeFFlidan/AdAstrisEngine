#define CORE_SHADER_BASE_MODEL_LAYOUT
#include "coreHF.hlsli"

PixelInput main(VertexInput input)
{
	PixelInput output;
	RendererModelInstance modelInstance = get_model_instance(input.instanceID);
	output.location = mul(modelInstance.transform.get_matrix(), float4(input.location, 1.0f));
	output.location = mul(get_camera().viewProjection, output.location);
	output.instanceID = input.instanceID;
	return output;
}
