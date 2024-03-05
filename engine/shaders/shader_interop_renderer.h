#ifndef SHADER_INTEROP_RENDERER
#define SHADER_INTEROP_RENDERER

#include "shader_interop_base.h"

// Many structs in this file are used with EntityManager as components. However, EntityManager is only used as a storage.
// Components from this file must not be used in application-side systems.

static const uint RENDERER_MATERIAL_FLAG_TRANSPARENT = 1 << 0;
static const uint RENDERER_MATERIAL_FLAG_UNLIT = 1 << 1;

enum TEXTURE_SLOT
{
	ALBEDO = 0,
	NORMAL,
	ROUGHNESS,
	METALLIC,
	AO,

	TEXTURE_SLOT_COUNT
};

struct RendererTexture2D
{
	int textureIndex;
	float3 empty;

	inline void init()
	{
		textureIndex = -1;
	}

	inline bool is_valid()
	{
		return textureIndex > 0;
	}

#ifndef __cplusplus
	Texture2D get_texture()
	{
		return bindlessTextures2DFloat4[NonUniformResourceIndex(textureIndex)];
	}

	float4 sample(in SamplerState sampler, in float4 uv_sets)
	{
		Texture2D texture = get_texture();
		float2 uv = uv_sets.xy;		// Have to think how to work with 2 uv sets
		return texture.Sample(sampler, uv);
	}
#endif
};

struct RendererMaterial
{
	uint2 albedo;	//half4
	uint roughness16Metallic16;
	uint opacity16SamplerIndex16;
	uint flags8;		// 24 bits are empty
	float3 empty;

	RendererTexture2D textures[TEXTURE_SLOT_COUNT];

#ifndef __cplusplus
	void init()
	{
		uint albedoDefaultValue = f32tof16(1.0f);
		albedo.x |= albedoDefaultValue;
		albedo.x |= albedoDefaultValue << 16u;
		albedo.y |= albedoDefaultValue;
		albedo.y |= albedoDefaultValue << 16u;
		roughness16Metallic16 = f32tof16(0.5f);
		opacity16SamplerIndex16 = f32tof16(1.0f);
		flags8 = 0;
 	}

	inline float4 get_albedo()
	{
		return float4(
			f16tof32(albedo.x),
			f16tof32(albedo.x >> 16u),
			f16tof32(albedo.y),
			f16tof32(albedo.y >> 16u)
		);
	}

	inline float get_roughness()
	{
		return f16tof32(roughness16Metallic16);
	}

	inline float get_metallic()
	{
		return f16tof32(roughness16Metallic16 >> 16u);
	}

	inline uint get_sampler_index()
	{
		return opacity16SamplerIndex16 >> 16u;
	}

	void get_sampler(out SamplerState sampler)
	{
		uint samplerIndex = opacity16SamplerIndex16 >> 16u;
		sampler = bindlessSamplers[NonUniformResourceIndex(samplerIndex)];
	}

	inline float get_opacity()
	{
		return f16tof32(opacity16SamplerIndex16);
	}
#else
	void init()
	{
		uint albedoDefaultValue = XMConvertFloatToHalf(1.0f);
		albedo.x |= albedoDefaultValue;
		albedo.x |= albedoDefaultValue << 16u;
		albedo.y |= albedoDefaultValue;
		albedo.y |= albedoDefaultValue << 16u;
		roughness16Metallic16 = XMConvertFloatToHalf(0.5f);
		opacity16SamplerIndex16 = XMConvertFloatToHalf(1.0f);
		flags8 = 0;
	}

	inline void set_albedo(float4 albedoVal)
	{
		albedo.x |= XMConvertFloatToHalf(albedoVal.x);
		albedo.x |= XMConvertFloatToHalf(albedoVal.y) << 16u;
		albedo.y |= XMConvertFloatToHalf(albedoVal.z);
		albedo.y |= XMConvertFloatToHalf(albedoVal.w) << 16u;
	}

	inline void set_roughness(float roughness)
	{
		roughness16Metallic16 |= XMConvertFloatToHalf(roughness);
	}

	inline void set_metallic(float metallic)
	{
		roughness16Metallic16 |= XMConvertFloatToHalf(metallic) << 16u;
	}

	inline void set_opacity(float opacity)
	{
		opacity16SamplerIndex16 |= XMConvertFloatToHalf(opacity);
	}

	inline void set_sampler_index(uint samplerIndex)
	{
		opacity16SamplerIndex16 |= samplerIndex << 16u;
	}

	inline void set_flags(uint flags)
	{
		flags8 |= (flags & 0xFF);
	}
#endif

	inline bool is_transparent()
	{
		uint flags = flags8 & 0xFF;
		return flags & RENDERER_MATERIAL_FLAG_TRANSPARENT;
	}

	inline bool is_unlit()
	{
		uint flags = flags8 & 0xFF;
		return flags & RENDERER_MATERIAL_FLAG_UNLIT;
	}
};

struct RendererTransform
{
	float4 location;
	float4 rotation;
	float4 scale;

	void init()
	{
		location = float4(1.0f, 0.0f, 0.0f, 0.0f);
		rotation = float4(0.0f, 1.0f, 0.0f, 0.0f);
		scale = float4(0.0f, 0.0f, 1.0f, 0.0f);
	}

	void set_transfrom(float4x4 mat)
	{
		location = float4(mat._11, mat._21, mat._31, mat._41);
		rotation = float4(mat._12, mat._22, mat._32, mat._42);
		scale = float4(mat._13, mat._23, mat._33, mat._43);
	}

	float4x4 get_matrix()
	{
		return float4x4(
			location.x, location.y, location.z, location.w,
			rotation.x, rotation.y, rotation.z, rotation.w,
			scale.x, scale.y, scale.z, scale.w,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}
};

struct RendererSphereBounds
{
	float3 center;
	float radius;
};

struct RendererModelInstance
{
	RendererTransform transform;
	RendererTransform transformInverseTranspose;
	
	RendererSphereBounds sphereBounds;

	uint materialIndex;
	float3 scale;
};

enum RendererEntityType
{
	DIRECTIONAL_LIGHT = 0,
	POINT_LIGHT,
	SPOT_LIGHT
};

// Must be aligned to 16 bytes. For now RendererEntity supports point, spot and directional lights
struct RendererEntity
{
	uint type8Flags8;					// 16 bits are empty
	float3 location;
	uint2 direction16OuterConeAngle16;	// There are 16 empty bits if RenderEntity type is dir light. I think it can be used in the future
	uint2 color;						// half4
	uint matrixIndex16TextureIndex16;
	uint attenuationRadius16;			// 16 bits are empty
	uint extraInfo16;					// Can contain two half floats. For spot light it will contain info about angles. For point light it will contain info about cubemap 
	uint empty;							// Will be used in the future

#ifndef __cplusplus
	inline uint get_type()
	{
		return type8Flags8 & 0xFF;
	}

	inline uint get_flags()
	{
		return (type8Flags8 >> 8) & 0xFF;
	}

	// Returns normalized direction
	inline float3 get_direction()
	{
		return normalize(float3(
			f16tof32(direction16OuterConeAngle16.x),
			f16tof32(direction16OuterConeAngle16.x >> 16u),
			f16tof32(direction16OuterConeAngle16.y)
		));
	}

	inline float get_outer_cone_angle()
	{
		return f16tof32(direction16OuterConeAngle16.y >> 16u);
	}

	inline float4 get_color()
	{
		return float4(
			f16tof32(color.x),
			f16tof32(color.x >> 16u),
			f16tof32(color.y),
			f16tof32(color.y >> 16u)
		);
	}

	inline uint get_matrix_index()
	{
		return matrixIndex16TextureIndex16 & 0xFFFF;
	}

	inline uint get_texture_index()
	{
		return matrixIndex16TextureIndex16 >> 16u;
	}

	inline float get_attenuation_radius()
	{
		return f16tof32(attenuationRadius16);
	}

	inline float get_angle_scale()
	{
		return f16tof32(extraInfo16);
	}

	inline float get_angle_offset()
	{
		return f16tof32(extraInfo16 >> 16u);
	}
#else
	inline void set_type(uint type)
	{
		type8Flags8 |= (type & 0xFF);
	}

	inline void set_flags(uint flags)
	{
		type8Flags8 |= (flags & 0xFF) << 8u;
	}

	inline void set_direction(float3 direction)
	{
		direction16OuterConeAngle16.x = XMConvertFloatToHalf(direction.x);
		direction16OuterConeAngle16.x = XMConvertFloatToHalf(direction.y) << 16u;
		direction16OuterConeAngle16.y = XMConvertFloatToHalf(direction.z);
	}

	inline void set_outer_cone_angle_cos(float angle)
	{
		direction16OuterConeAngle16.y |= XMConvertFloatToHalf(angle) << 16u;
	}

	inline void set_color(float4 colorValue)
	{
		color.x |= XMConvertFloatToHalf(colorValue.x);
		color.x |= XMConvertFloatToHalf(colorValue.y) << 16u;
		color.y |= XMConvertFloatToHalf(colorValue.z);
		color.y |= XMConvertFloatToHalf(colorValue.w) << 16u;
	}

	inline void set_indices(uint matrixIndex, uint textureIndex)
	{
		matrixIndex16TextureIndex16 |= (matrixIndex & 0xFFFF);
		matrixIndex16TextureIndex16 |= (textureIndex & 0xFFFF) << 16u;
	}

	inline void set_attenuation_radius(float radius)
	{
		attenuationRadius16 |= XMConvertFloatToHalf(radius);
	}

	// For spot lights
	inline void set_angle_scale(float angleScale)
	{
		extraInfo16 |= XMConvertFloatToHalf(angleScale);
	}

	// For spot lights
	inline void set_angle_offset(float angleOffset)
	{
		extraInfo16 |= XMConvertFloatToHalf(angleOffset) << 16u;
	}
#endif	// __cplusplus
};

static const uint FRAME_FLAG_TEMPORAL_AA_ENABLED = 1 << 0;

struct FrameUB
{
	uint flags;
	float time;
	float previousTime;
	float deltaTime;

	uint frameCount;
	uint2 shadowMapAtlasExtent;
	int shadowMapAtlasIndex;

	int entityBufferIndex;
	int materialBufferIndex;
	int modelInstanceBufferIndex;
	int modelInstanceIDBufferIndex;

	uint lightArrayOffset;
	uint lightArrayCount;
	uint empty2;
	uint empty3;
};

struct RendererFrustum
{
	float4 planes[6];
#ifndef __cplusplus
	bool check(RendererSphereBounds sphereBounds)
	{
		bool visible = true;
		for (int i = 0; i != 6; ++i)
		{
			visible = visible && dot(planes[i], float4(sphereBounds.center, 1.0f)) > -sphereBounds.radius;
		}
		return visible;
	}
#endif
};

struct RendererCamera
{
	float3 location;
	uint empty1;

	float4 clipPlane;

	float3 forward;
	float zNear;

	float3 up;
	float zFar;

	float4x4 view;
	float4x4 projection;
	float4x4 viewProjection;
	float4x4 inverseView;
	float4x4 inverseProjection;
	float4x4 inverseViewProjection;

	RendererFrustum frustum;

#ifdef __cplusplus
	void create_frustum()
	{
		XMMATRIX projectionViewT = XMMatrixTranspose(XMLoadFloat4x4(&viewProjection));
		XMStoreFloat4(&frustum.planes[0], XMPlaneNormalize(projectionViewT.r[3] + projectionViewT.r[0]));
		XMStoreFloat4(&frustum.planes[1], XMPlaneNormalize(projectionViewT.r[3] - projectionViewT.r[0]));
		XMStoreFloat4(&frustum.planes[2], XMPlaneNormalize(projectionViewT.r[3] + projectionViewT.r[1]));
		XMStoreFloat4(&frustum.planes[3], XMPlaneNormalize(projectionViewT.r[3] - projectionViewT.r[1]));
		XMStoreFloat4(&frustum.planes[4], XMPlaneNormalize(projectionViewT.r[2]));
		XMStoreFloat4(&frustum.planes[5], XMPlaneNormalize(projectionViewT.r[3] - projectionViewT.r[2]));
	}
#endif
};

static const uint MAX_CAMERA_COUNT = 16;

struct CameraUB
{
	RendererCamera cameras[MAX_CAMERA_COUNT];
};

UNIFORM_BUFFER(frameData, FrameUB, UB_FRAME_SLOT);
UNIFORM_BUFFER(cameraData, CameraUB, UB_CAMERA_SLOT);

struct CullingDataUB
{
	float P00, P11, znear, zfar;
};

struct Attachments
{
	uint gAlbedoIndex;
	uint gNormalIndex;
	uint gSurfaceIndex;
	uint gDepthIndex;
};

struct IDColor
{
	float4 color;
};

enum SamplerType
{
	SAMPLER_LINEAR_REPEAT = 0,
	SAMPLER_LINEAR_CLAMP,
	SAMPLER_LINEAR_MIRROR,

	SAMPLER_NEAREST_REPEAT,
	SAMPLER_NEAREST_CLAMP,
	SAMPLER_NEAREST_MIRROR,

	SAMPLER_MINIMUM_NEAREST_CLAMP,

	SAMPLER_COUNT
};

// Indirect drawing
struct RendererModelInstanceID
{
	uint id;
	uint3 empty;
};

struct DrawIndexedIndirectCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

struct CullingInstanceIndices
{
	uint objectID;
	uint batchID;
};

struct IndirectBufferIndices
{
	uint cullingInstanceIndicesBufferIndex;
	uint indirectCommandsBufferIndex;
	uint rendererModelInstanceIdBufferIndex;
	uint cullingParamsBufferIndex;
	uint cullingParamsIndex;
	float2 empty1;
};

struct CullingParams
{
	uint cameraIndex;
	uint depthPyramidIndex;
	float lodBase, lodStep;
	float pyramidWidth, pyramidHeight;

	uint drawCount;

	int isFrustumCullingEnabled;
	int isOcclusionCullingEnabled;
	int isAABBCheckEnabled;
	int isLodEnabled;
	float aabbmin_x;
	float aabbmin_y;
	float aabbmin_z;
	float aabbmax_x;
	float aabbmax_y;
	float aabbmax_z;
	float3 empty;
};

static const uint CULLING_GROUP_SIZE = 256;

struct DepthReduceData
{
	uint levelWidth;
	uint levelHeight;
	uint inDepthTextureIndex;
	uint outDepthTextureIndex;
};

static const uint DEPTH_REDUCE_GROUP_SIZE = 32;

#endif // SHADER_INTEROP_RENDERER
