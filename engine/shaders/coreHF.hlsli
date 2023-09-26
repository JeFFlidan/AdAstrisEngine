#ifndef CORE_HF
#define CORE_HF
#include "common.hlsli"

#ifdef CORE_SHADER_BASE_MODEL_LAYOUT
#define CORE_SHADER_USE_NORMAL
#define CORE_SHADER_USE_TANGENT
#define CORE_SHADER_USE_UVSETS
#endif

struct VertexInput
{
    float3 location : SV_POSITION;
    uint instanceID : SV_InstanceID;
    
#ifdef CORE_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif
    
#ifdef CORE_SHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif
    
#ifdef CORE_SHADER_USE_UVSETS
    float2 texCoord : TEXCOORD0;
#endif
};

struct PixelInput
{
    precise float4 location : SV_POSITION;
    
#ifdef CORE_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif
    
#ifdef CORE_SHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif
    
#ifdef CORE_SHADER_USE_UVSETS
    float2 texCoord : TEXCOORD0;
#endif
};

struct VertexDesc
{
    float4 location;
    float2 uvSets;
    float3 normal;
    float3 tangent;
    
    void init(in VertexInput input)
    {
        RendererModelInstance modelInstance = get_model_instance(input.instanceID);
        location = mul(modelInstance.transform.get_matrix(), float4(input.location, 1.0f));
        
#ifdef CORE_SHADER_USE_UVSETS
        uvSets = input.texCoord;
#endif
        
#ifdef CORE_SHADER_USE_NORMAL
        normal = mul((float3x3) modelInstance.transformInverseTranspose.get_matrix(), input.normal);
#endif
        
#ifdef CORE_SHADER_USE_TANGENT
        tangent = mul((float3x3) modelInstance.transformInverseTranspose.get_matrix(), input.tangent);
#endif
    }
};

#ifdef CORE_SHADER_COMPILE_VS
PixelInput main(VertexInput input)
{
    PixelInput output;
    
    // Think about instance index in PixelInput
    VertexDesc vertexDesc;
    vertexDesc.init(input);
    
    output.location = vertexDesc.location;
    
#ifdef CORE_SHADER_USE_CAMERA
    output.location = mul(get_camera().viewProjection, output.location);
#endif
    
#ifdef CORE_SHADER_USE_UVSETS
    output.texCoord = vertexDesc.uvSets;
#endif
        
#ifdef CORE_SHADER_USE_NORMAL
    output.normal = vertexDesc.normal;
#endif
        
#ifdef CORE_SHADER_USE_TANGENT
    output.tangent = vertexDesc.tangent;
#endif
    
    return output;
}
#endif


#ifdef CORE_SHADER_COMPILE_PS_GBUFFER
struct GBufferOutput
{
    float4 gAlbedo : SV_Target0;
	float4 gNormal : SV_Target1;
	float4 gSurface : SV_Target2;
};
#endif

/* Switches:
*   CORE_SHADER_COMPILE_PS_GBUFFER     -  compile core pixel shader for gbuffer rendering
*   CORE_SHADER_COMPILE_PS_TRANSPARENT -  compile core pixel shader for order independent transparent rendering
*/

#ifdef CORE_SHADER_COMPILE_PS

#ifdef CORE_SHADER_USE_EARLY_DEPTH_TEST
[earlydepthstencil]
#endif

#ifdef CORE_SHADER_COMPILE_PS_PREPASS
uint main(PixelInput input) : SV_Target
#else
float4 main(PixelInput input) : SV_Target
{
    
}
#endif
#endif

#endif // CORE_HF