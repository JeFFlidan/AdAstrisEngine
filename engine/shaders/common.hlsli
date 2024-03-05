#ifndef SHADER_COMMON
#define SHADER_COMMON

#include "shader_interop_base.h"

#if defined(__spirv__)
   
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER = 1;
static const uint BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER = 2;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER = 3;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE = 4;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE = 5;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLER = 6;

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] ByteAddressBuffer bindlessBuffers[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWByteAddressBuffer bindlessRWBuffers[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint> bindlessBuffersUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint2> bindlessBuffersUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint3> bindlessBuffersUint3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint4> bindlessBuffersUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float> bindlessBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float2> bindlessBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float3> bindlessBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float4> bindlessBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint> bindlessRWBuffersUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint2> bindlessRWBuffersUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint3> bindlessRWBuffersUint3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint4> bindlessRWBuffersUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float> bindlessRWBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float2> bindlessRWBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float3> bindlessRWBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float4> bindlessRWBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D bindlessTextures2D[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float> bindlessTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float2> bindlessTextures2DFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float4> bindlessTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint> bindlessTextures2DUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint2> bindlessTextures2DUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint4> bindlessTextures2DUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2DArray<float4> bindlessTexture2DArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCube bindlessCubemaps[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCubeArray bindlessCubemapsArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture3D bindlessTextures3D[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float> bindlessRWTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<uint> bindlessRWTextures2DUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2DArray<float4> bindlessRWTexture2DArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<float4> bindlessRWTextures3D[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLER)]] SamplerState bindlessSamplers[];

#endif

#include "shader_interop_renderer.h"

#if defined(__spirv__)
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<RendererModelInstance> bindlessStructuredModelInstances[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<RendererModelInstanceID> bindlessStructuredModelInstanceIDs[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<RendererMaterial> bindlessStructuredMaterials[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<RendererEntity> bindlessStructuredEntities[];
#endif

inline FrameUB get_frame()
{
    return frameData;
}

inline RendererCamera get_camera(uint cameraIndex = 0)
{
    if (cameraIndex < MAX_CAMERA_COUNT)
        return cameraData.cameras[cameraIndex];
    return cameraData.cameras[0];
}

inline RendererEntity get_entity(uint entityIndex)
{
    return bindlessStructuredEntities[get_frame().entityBufferIndex][entityIndex];
}

inline RendererMaterial get_material(uint materialIndex)
{
    return bindlessStructuredMaterials[get_frame().materialBufferIndex][materialIndex];
}

inline RendererModelInstanceID get_model_instance_id(uint hlslInstanceID)
{
    return bindlessStructuredModelInstanceIDs[get_frame().modelInstanceIDBufferIndex][hlslInstanceID];
}

inline RendererModelInstance get_model_instance(uint hlslInstanceID)
{
    return bindlessStructuredModelInstances[get_frame().modelInstanceBufferIndex][get_model_instance_id(hlslInstanceID).id];
}

inline float3 get_world_position_from_depth(float depth, float2 texCoord, float cameraIndex = 0)
{
    float x = texCoord.x * 2 - 1;
    float y = (1 - texCoord.y) * 2 - 1;
    float4 clipSpaceLocation = float4(x, y, depth, 1.0);
    float4 worldSpaceLocation = mul(get_camera(cameraIndex).inverseViewProjection, clipSpaceLocation);
    return worldSpaceLocation.xyz / worldSpaceLocation.w;
}

static const float PI = 3.14159265359;

#endif  // SHADER_COMMON
