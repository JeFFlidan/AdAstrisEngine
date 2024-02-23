#ifndef CORE_HF
#define CORE_HF
#include "common.hlsli"
#include "lightingHF.hlsli"

#ifdef CORE_SHADER_BASE_MODEL_LAYOUT
#define CORE_SHADER_USE_NORMAL
#define CORE_SHADER_USE_TANGENT
#define CORE_SHADER_USE_TEXCOORD
#endif

#ifdef CORE_SHADER_OUTPUT_PLANE
#define CORE_SHADER_USE_TEXCOORD
#define CORE_SHADER_USE_VERTEX_ID
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
    
#ifdef CORE_SHADER_USE_TEXCOORD
    float2 texCoord : TEXCOORD0;
#endif

#ifdef CORE_SHADER_USE_UV_SETS
    float4 uvSets : UVSETS;
#endif

#ifdef CORE_SHADER_USE_VERTEX_ID
    uint vertexID : SV_VertexID;
#endif
};

struct PixelInput
{
    precise float4 location : SV_POSITION;
    uint instanceID : SV_InstanceID;
    
#ifdef CORE_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif
    
#ifdef CORE_SHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif
    
#ifdef CORE_SHADER_USE_TEXCOORD
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
        
#ifdef CORE_SHADER_USE_TEXCOORD
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
    output.instanceID = input.instanceID;
    
#ifdef CORE_SHADER_USE_CAMERA
    output.location = mul(get_camera().viewProjection, output.location);
#endif
    
#ifdef CORE_SHADER_USE_TEXCOORD
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
#endif
#ifdef CORE_SHADER_COMPILE_PS_GBUFFER
GBufferOutput main(PixelInput input)
#else

PUSH_CONSTANT(attachments, Attachments);

float4 main(PixelInput input) : SV_Target
#endif
{
#ifdef CORE_SHADER_COMPILE_PS_GBUFFER
    GBufferOutput output;
    RendererModelInstance modelInstance = get_model_instance(input.instanceID);
    RendererMaterial material = get_material(modelInstance.materialIndex);
    SamplerState sampler = bindlessSamplers[material.get_sampler_index()];
    output.gAlbedo = material.textures[ALBEDO].get_texture().Sample(sampler, input.texCoord);
    float aoVal = material.textures[AO].get_texture().Sample(sampler, input.texCoord).r;
    float roughVal = material.textures[ROUGHNESS].get_texture().Sample(sampler, input.texCoord).r;
    float metallicVal = material.textures[METALLIC].get_texture().Sample(sampler, input.texCoord).r;
    output.gSurface = float4(aoVal, roughVal, metallicVal, 1.0f);

    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    float3 normal = material.textures[NORMAL].get_texture().Sample(sampler, input.texCoord).xyz;
    normal = normalize(mul(TBN, normalize(2.0 * normal - 1.0)));
    output.gNormal = float4(normal, 1.0f);
    output.gNormal = normalize(output.gNormal);

    return output;
#endif
#ifdef CORE_SHADER_DEFERRED_LIGHTING
    SurfaceDesc surface;
    SamplerState sampler = bindlessSamplers[SAMPLER_LINEAR_CLAMP];
    float depth = bindlessTextures2D[NonUniformResourceIndex(attachments.gDepthIndex)].Sample(sampler, input.texCoord).r;
    surface.location = get_world_position_from_depth(depth, input.texCoord);

    surface.albedo = bindlessTextures2D[NonUniformResourceIndex(attachments.gAlbedoIndex)].Sample(sampler, input.texCoord);
    surface.normal = bindlessTextures2D[NonUniformResourceIndex(attachments.gNormalIndex)].Sample(sampler, input.texCoord).rgb;
    surface.roughness = bindlessTextures2D[NonUniformResourceIndex(attachments.gSurfaceIndex)].Sample(sampler, input.texCoord).g;
    surface.metallic = bindlessTextures2D[NonUniformResourceIndex(attachments.gSurfaceIndex)].Sample(sampler, input.texCoord).b;
    surface.ao = bindlessTextures2D[NonUniformResourceIndex(attachments.gSurfaceIndex)].Sample(sampler, input.texCoord).r;

    surface.view = normalize(get_camera().location - surface.location);

    float3 dirLightsL0 = float3(0.0f, 0.0f, 0.0f);
    float3 pointLightsL0 = float3(0.0f, 0.0f, 0.0f);
    float3 spotLightsL0 = float3(0.0f, 0.0f, 0.0f);

    uint lightArrayOffset = get_frame().lightArrayOffset;
    uint lightCount = get_frame().lightArrayCount;

    for (uint i = lightArrayOffset; i != lightArrayOffset + lightCount; ++i)
    {
        RendererEntity entity = get_entity(i);
        switch (entity.get_type())
        {
            case POINT_LIGHT:
            {
                pointLightsL0 += calculate_point_light(entity, surface);
                break;
            }
            case DIRECTIONAL_LIGHT:
            {
                dirLightsL0 += calculate_directional_light(entity, surface);
                //dirLightsL0 = max(dot(entity.get_direction(), surface.normal), 0.0);
                //dirLightsL0 = normalize(normalize(entity.get_direction()) + surface.view);
                break;
            }
            case SPOT_LIGHT:
            {
                spotLightsL0 += calculate_spot_light(entity, surface);
                break;
            }
        }
    }

    //float3 ambient = float3(0.01, 0.01, 0.01) * surface.albedo.xyz * surface.ao;
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 finalColor = dirLightsL0 + spotLightsL0 + pointLightsL0 + ambient;
    return float4(finalColor.x, finalColor.y, finalColor.z, 1.0f);
#endif
}

#endif
#endif // CORE_HF