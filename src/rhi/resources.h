#pragma once
#include <cstdint>

namespace ad_astris::rhi
{
	enum ResourceUsage
	{
		UNDEFINED_USAGE,
		SAMPLED_TEXTURE,
		STORAGE_TEXTURE,
		COLOR_ATTACHMENT,
		DEPTH_STENCIL_ATTACHMENT,
		TRANSIENT_ATTACHMENT,
		INPUT_ATTACHMENT,
		UNIFORM_TEXEL_BUFFER,
		STORAGE_TEXEL_BUFFER,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		INDEX_BUFFER,
		VERTEX_BUFFER,
		INDIRECT_BUFFER
	};

	enum MemoryUsage
	{
		UNDEFINED_MEMORY_USAGE,
		CPU,
		GPU,
		CPU_TO_GPU
	};
	
	enum TextureFormat
	{
		UNDEFINED_FORMAT = 0,
		R4G4_UNORM,
		R4G4B4A4_UNORM,
		B4G4R4A4_UNORM,
		R5G5B5A1_UNORM,
		B5G5R5A1_UNORM,
		A1R5G5B5_UNORM,
	
		R8_UNORM,
		R8_SNORM,
		R8_UINT,
		R8_SINT,
		R8_SRGB,
	
		R8G8_UNORM,
		R8G8_SNORM,
		R8G8_UINT,
		R8G8_SINT,
		R8G8_SRGB,

		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R8G8B8A8_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_SRGB,

		B8G8R8A8_SRGB,
		B8G8R8A8_UNORM,
		B8G8R8A8_SNORM,

		R10G10B10A2_UNORM,
		R10G10B10A2_SNORM,

		R16_UNORM,
		R16_SNORM,
		R16_UINT,
		R16_SINT,
		R16_SFLOAT,

		R16G16_UNORM,
		R16G16_SNORM,
		R16G16_UINT,
		R16G16_SINT,
		R16G16_SFLOAT,

		R16G16B16A16_UNORM,
		R16G16B16A16_SNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SINT,
		R16G16B16A16_SFLOAT,

		R32_UINT,
		R32_SINT,
		R32_SFLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G32_SFLOAT,

		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32A32_SFLOAT,

		D16_UNORM,
		D32_SFLOAT,

		S8_UINT,
		D16_UNORM_S8_UINT,
		D24_UNORM_S8_UINT,
		D32_SFLOAT_S8_UINT,
	};

	enum AddressMode
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRROR_CLAMP_TO_EDGE
	};

	// Base on D3D12
	enum Filter
	{
		MIN_MAG_MIP_NEAREST,
		MIN_MAG_NEAREST_MIP_LINEAR,
		MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		MIN_NEAREST_MAG_MIP_LINEAR,
		MIN_LINEAR_MAG_MIP_NEAREST,
		MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
		MIN_MAG_LINEAR_MIP_NEAREST,
		MIN_MAG_MIP_LINEAR,
		ANISOTROPIC,
		COMPARISON_MIN_MAG_MIP_NEAREST,
		COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR,
		COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR,
		COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST,
		COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
		COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST,
		COMPARISON_MIN_MAG_MIP_LINEAR,
		COMPARISON_ANISOTROPIC,
		MINIMUM_MIN_MAG_MIP_NEAREST,
		MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR,
		MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR,
		MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST,
		MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
		MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST,
		MINIMUM_MIN_MAG_MIP_LINEAR,
		MINIMUM_ANISOTROPIC,
		MAXIMUM_MIN_MAG_MIP_NEAREST,
		MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR,
		MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR,
		MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST,
		MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
		MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST,
		MAXIMUM_MIN_MAG_MIP_LINEAR,
		MAXIMUM_ANISOTROPIC
	};

	enum SampleCount
	{
		SAMPLE_COUNT_1_BIT,
		SAMPLE_COUNT_2_BIT,
		SAMPLE_COUNT_4_BIT,
		SAMPLE_COUNT_8_BIT,
		SAMPLE_COUNT_16_BIT,
		SAMPLE_COUNT_32_BIT,
		SAMPLE_COUNT_64_BIT
	};

	enum TextureDimension
	{
		TEXTURE1D,
		TEXTURE2D,
		TEXTURE3D
	};

	enum BorderColor
	{
		FLOAT_TRANSPARENT_BLACK,
		INT_TRANSPARENT_BLACK,
		FLOAT_OPAQUE_BLACK,
		INT_OPAQUE_BLACK,
		FLOAT_OPAQUE_WHITE,
		INT_OPAQUE_WHITE
	};

	enum ShaderType
	{
		UNDEFINED_SHADER_TYPE,
		
		VERTEX,
		FRAGMENT,
		TESSELLATION_CONTROL,
		TESSELLATION_EVALUATION,
		GEOMETRY,

		COMPUTE,

		MESH,
		TASK,

		RAY_GENERATION,
		RAY_INTERSECTION,
		RAY_ANY_HIT,
		RAY_CLOSEST_HIT,
		RAY_MISS,
		RAY_CALLABLE
	};

	struct TextureInfo
	{
		TextureInfo() = default;
		TextureInfo(uint32_t width, uint32_t height, TextureFormat format, ResourceUsage usage, bool transSrc, bool transDst)
			: width(width), height(height), format(format), textureUsage(usage), transferSrc(transSrc), transferDst(transDst) {}
		
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		uint32_t layersCount{ 1 };
		TextureFormat format{ UNDEFINED_FORMAT };
		ResourceUsage textureUsage{ UNDEFINED_USAGE };
		MemoryUsage memoryUsage{ GPU };
		SampleCount samplesCount{ SAMPLE_COUNT_1_BIT };
		TextureDimension textureDimension{ TEXTURE2D };
		bool transferSrc{ false };
		bool transferDst{ false };
	};
	

	struct BufferInfo
	{
		BufferInfo() = default;
		BufferInfo(ResourceUsage bufferUsage, MemoryUsage memoryUsage, bool transSrc, bool transDst)
			: bufferUsage(bufferUsage), memoryUsage(memoryUsage), transferSrc(transSrc), transferDst(transDst) {}
		
		ResourceUsage bufferUsage{ UNDEFINED_USAGE };
		MemoryUsage memoryUsage{ UNDEFINED_MEMORY_USAGE };
		bool transferSrc{ false };
		bool transferDst{ false };
	};
	
	struct Resource
	{
		enum class ResourceType
		{
			BUFFER,
			TEXTURE,
			UNDEFINED_TYPE
		} type = ResourceType::UNDEFINED_TYPE;

		void* data{ nullptr };		// Pointer to Vulkan or D3D12 buffer 
		uint64_t size{ 0 };		// Size in bytes

		bool is_buffer() { return type == ResourceType::BUFFER; }
		bool is_texture() { return type == ResourceType::TEXTURE; }
		bool is_undefined() { return type == ResourceType::UNDEFINED_TYPE; }
		bool is_valid() { return data && size; }
	};

	struct Buffer : public Resource
	{
		Buffer() = default;
		Buffer(BufferInfo info) : bufferInfo(info) {}
		BufferInfo bufferInfo;
	};

	struct Texture : public Resource
	{
		Texture() = default;
		Texture(TextureInfo info) : textureInfo(info) {}
		TextureInfo textureInfo;
	};

	struct ObjectHandle
	{
		void* handle{ nullptr };
		bool is_valid() { return handle; }
	};
	
	struct SamplerInfo
	{
		Filter filter{ MIN_MAG_MIP_LINEAR };
		AddressMode addressMode{ REPEAT };
		BorderColor borderColor{ FLOAT_OPAQUE_WHITE };
		float minLod{ 0.0f };
		float maxLod{ 1.0f };
		float maxAnisotropy{ 1.0f };
	};

	struct Sampler : public ObjectHandle
	{
		Sampler(SamplerInfo info) : info(info) {}
		SamplerInfo info;
	};

	struct TextureViewInfo
	{
		uint8_t baseMipLevel{ 0 };
		uint8_t baseLayer{ 0 };
	};

	struct TextureView : public ObjectHandle
	{
		TextureView(TextureViewInfo info) : viewInfo(info) {}
		TextureViewInfo viewInfo;
	};

	struct ShaderInfo
	{
		ShaderType shaderType{ UNDEFINED_SHADER_TYPE };
		uint32_t* data{ nullptr };		// Pointer to SPIRV or DXIL data (depends on chosen API)
		uint64_t size{ 0 };
	};

	struct Shader : public ObjectHandle
	{
		ShaderType type;
	};
}
