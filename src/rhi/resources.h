#pragma once
#include "flags_operations.h"
#include <cstdint>
#include <vector>

namespace ad_astris::rhi
{
	enum class ResourceFlags
	{
		CUBE_TEXTURE = 1 << 0,
	};
	
	enum class LogicOp
	{
		UNDEFINED,
		CLEAR,
		AND,
		AND_REVERSE,
		COPY,
		AND_INVERTED,
		NO_OP,
		XOR,
		OR,
		NOR,
		EQUIVALENT,
		INVERT,
		OR_REVERSE,
		COPY_INVERTED,
		OR_INVERTED,
		NAND,
		SET,
	};

	enum class ResourceLayout
	{
		UNDEFINED = 1 << 0,
		GENERAL = 1 << 1,
		SHADER_READ = 1 << 2,
		SHADER_WRITE = 1 << 3,
		MEMORY_READ = 1 << 4,
		MEMORY_WRITE = 1 << 5,
		TRANSFER_SRC = 1 << 6,
		TRANSFER_DST = 1 << 7,

		COLOR_ATTACHMENT = 1 << 8,
		DEPTH_STENCIL = 1 << 9,
		DEPTH_STENCIL_READ_ONLY = 1 << 10,

		INDIRECT_COMMAND_BUFFER = 1 << 11,
		VERTEX_BUFFER = 1 << 12,
		INDEX_BUFFER = 1 << 13,
		UNIFORM_BUFFER = 1 << 14
	};
	
	enum class ResourceUsage
	{
		UNDEFINED = 1 << 0,
		TRANSFER_SRC = 1 << 1,
		TRANSFER_DST = 1 << 2,
		SAMPLED_TEXTURE = 1 << 3,
		STORAGE_TEXTURE = 1 << 4,
		COLOR_ATTACHMENT = 1 << 5,
		DEPTH_STENCIL_ATTACHMENT = 1 << 6,
		TRANSIENT_ATTACHMENT = 1 << 7,
		INPUT_ATTACHMENT = 1 << 8,
		UNIFORM_TEXEL_BUFFER = 1 << 10,
		STORAGE_TEXEL_BUFFER = 1 << 11,
		UNIFORM_BUFFER = 1 << 12,
		STORAGE_BUFFER = 1 << 13,
		INDEX_BUFFER = 1 << 14,
		VERTEX_BUFFER = 1 << 15,
		INDIRECT_BUFFER = 1 << 16
	};

	enum class MemoryUsage
	{
		UNDEFINED,
		CPU,
		GPU,
		CPU_TO_GPU
	};
	
	enum class Format
	{
		UNDEFINED = 0,
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

		R32G32B32_UINT,
		R32G32B32_SINT,
		R32G32B32_SFLOAT,
		
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

	enum class AddressMode
	{
		UNDEFINED,
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRROR_CLAMP_TO_EDGE
	};

	// Base on D3D12
	enum class Filter
	{
		UNDEFINED,
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

	enum class SampleCount
	{
		UNDEFINED,
		BIT_1,
		BIT_2,
		BIT_4,
		BIT_8,
		BIT_16,
		BIT_32,
		BIT_64
	};

	enum class TextureDimension
	{
		UNDEFINED,
		TEXTURE1D,
		TEXTURE2D,
		TEXTURE3D
	};

	enum class BorderColor
	{
		UNDEFINED,
		FLOAT_TRANSPARENT_BLACK,
		INT_TRANSPARENT_BLACK,
		FLOAT_OPAQUE_BLACK,
		INT_OPAQUE_BLACK,
		FLOAT_OPAQUE_WHITE,
		INT_OPAQUE_WHITE
	};

	enum class ShaderType
	{
		UNDEFINED,
		
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
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		uint32_t layersCount{ 1 };
		Format format{ Format::UNDEFINED };
		ResourceUsage textureUsage{ ResourceUsage::UNDEFINED };
		MemoryUsage memoryUsage{ MemoryUsage::UNDEFINED };
		SampleCount samplesCount{ SampleCount::UNDEFINED };
		TextureDimension textureDimension{ TextureDimension::UNDEFINED };
		ResourceFlags resourceFlags;	// not necessary
	};

	struct BufferInfo
	{
		ResourceUsage bufferUsage{ ResourceUsage::UNDEFINED };
		MemoryUsage memoryUsage{ MemoryUsage::UNDEFINED };
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
		BufferInfo bufferInfo;
	};

	template<typename T>
	struct TypedBuffer : public Buffer
	{
		
	};

	struct Texture : public Resource
	{
		TextureInfo textureInfo;
	};

	struct ObjectHandle
	{
		void* handle{ nullptr };
		bool is_valid() { return handle; }
	};

	struct SwapChainInfo
	{
		bool sync;
		uint8_t buffersCount;
		uint32_t width;
		uint32_t height;
	};

	struct SwapChain : public ObjectHandle
	{

	};
	
	struct SamplerInfo
	{
		Filter filter{ Filter::UNDEFINED};
		AddressMode addressMode{ AddressMode::UNDEFINED };
		BorderColor borderColor{ BorderColor::UNDEFINED };
		float minLod{ 0.0f };
		float maxLod{ 1.0f };
		float maxAnisotropy{ 1.0f };
	};

	struct Sampler : public ObjectHandle
	{
		SamplerInfo sampInfo;
	};

	struct TextureViewInfo
	{
		uint8_t baseMipLevel;
		uint8_t baseLayer;
	};

	struct TextureView : public ObjectHandle
	{
		TextureViewInfo viewInfo;
		Texture* texture{ nullptr };
	};

	struct ShaderInfo
	{
		ShaderType shaderType{ ShaderType::UNDEFINED };
		uint8_t* data{ nullptr };		// Pointer to SPIRV or DXIL data (depends on chosen API)
		uint64_t size{ 0 };
	};

	struct Shader : public ObjectHandle
	{
		ShaderType type{ ShaderType::UNDEFINED };
	};

	enum class LoadOp
	{
		LOAD,
		CLEAR,
		DONT_CARE,
	};

	enum class StoreOp
	{
		STORE,
		DONT_CARE,
	};

	enum class RenderTargetType
	{
		DEPTH,
		COLOR,
	};

	struct RenderTarget
	{
		TextureView* target;
		RenderTargetType type;
		LoadOp loadOp;
		StoreOp storeOp;
		ResourceLayout initialLayout;
		ResourceLayout renderPassLayout;
		ResourceLayout finalLayout;
	};

	enum class PipelineType
	{
		UNDEFINED,
		GRAPHICS,
		COMPUTE,
		RAY_TRACING
	};

	struct MultiviewInfo
	{
		bool isEnabled{ false }; // false is default
		uint32_t viewCount{ 0 };
	};

	struct RenderPassInfo
	{
		std::vector<RenderTarget> renderTargets;
		PipelineType pipelineType{ PipelineType::UNDEFINED };
		MultiviewInfo multiviewInfo;	// not necessary
	};
	
	struct RenderPass : public ObjectHandle
	{
		
	};
	
	enum class TopologyType
	{
		UNDEFINED,
		POINT,
		LINE,
		TRIANGLE,
		PATCH,
	};

	struct AssemblyState
	{
		TopologyType topologyType{ TopologyType::UNDEFINED };
	};

	/**
	 * POLYGON_MODE_POINT available only if you use Vulkan
	 */
	enum class PolygonMode
	{
		UNDEFINED,
		FILL,
		LINE,
		POINT
	};

	/**
	 * CULL_MODE_FRONT_AND_BACK available only if you use Vulkan
	 */
	enum class CullMode
	{
		UNDEFINED,
		NONE,
		FRONT,
		BACK,
		FRONT_AND_BACK
	};

	enum class FrontFace
	{
		UNDEFINED,
		CLOCKWISE,
		COUNTER_CLOCKWISE
	};

	struct RasterizationState
	{
		PolygonMode polygonMode{ PolygonMode::UNDEFINED };
		CullMode cullMode{ CullMode::UNDEFINED };
		FrontFace frontFace{ FrontFace::UNDEFINED };
		bool isBiasEnabled;
		float lineWidth{ 1.0f };
	};

	struct VertexBindingDescription
	{
		uint32_t binding;
		uint32_t stride;
	};

	struct VertexAttributeDescription
	{
		uint32_t binding;
		uint32_t location;
		uint32_t offset;
		Format format{ Format::UNDEFINED };
	};

	struct MultisampleState
	{
		SampleCount sampleCount{ SampleCount::UNDEFINED };
		bool isEnabled;
	};

	enum class BlendFactor
	{
		UNDEFINED,
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA,
	};

	enum class BlendOp
	{
		UNDEFINED,
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX,
	};

	struct ColorBlendAttachmentState
	{
		bool isBlendEnabled;
		BlendFactor srcColorBlendFactor{ BlendFactor::UNDEFINED };
		BlendFactor dstColorBlendFactor{ BlendFactor::UNDEFINED};
		BlendOp colorBlendOp{ BlendOp::UNDEFINED };
		BlendFactor srcAlphaBlendFactor{ BlendFactor::UNDEFINED };
		BlendFactor dstAlphaBlendFactor{ BlendFactor::UNDEFINED };
		BlendOp alphaBlendOp{ BlendOp::UNDEFINED };
		uint64_t colorWriteMask{ 0xF };
	};

	struct ColorBlendState
	{
		bool isLogicOpEnabled;
		LogicOp logicOp{ LogicOp::UNDEFINED };
		std::vector<ColorBlendAttachmentState> colorBlendAttachments;
	};

	enum class CompareOp
	{
		UNDEFINED,
		NEVER,
		LESS,
		EQUAL,
		LESS_OR_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_OR_EQUAL,
		ALWAYS,
	};

	enum class StencilOp
	{
		UNDEFINED,
		KEEP,
		ZERO,
		REPLACE,
		INCREMENT_AND_CLAMP,
		DECREMENT_AND_CLAMP,
		INVERT,
		INCREMENT_AND_WRAP,
		DECREMENT_AND_WRAP,
	};

	struct StencilOpState
	{
		StencilOp failOp{ StencilOp::UNDEFINED };
		StencilOp passOp{ StencilOp::UNDEFINED };
		StencilOp depthFailOp{ StencilOp::UNDEFINED };
		CompareOp compareOp{ CompareOp::UNDEFINED };
		uint32_t compareMask;
		uint32_t writeMask;
		uint32_t reference;
	};
	
	struct DepthStencilState
	{
		bool isDepthTestEnabled;
		bool isDepthWriteEnabled;
		CompareOp compareOp{ CompareOp::UNDEFINED };
		bool isStencilTestEnabled;
		StencilOpState frontStencil;
		StencilOpState backStencil;
	};
	
	struct GraphicsPipelineInfo
	{
		AssemblyState assemblyState;
		RasterizationState rasterizationState;
		MultisampleState multisampleState;
		ColorBlendState colorBlendState;
		DepthStencilState depthStencilState;
		std::vector<Shader> shaderStages;
		std::vector<VertexBindingDescription> bindingDescriptrions;
		std::vector<VertexAttributeDescription> attributeDescriptions;
		RenderPass renderPass;
	};

	struct ComputePipelineInfo
	{
		Shader shaderStage;
	};

	struct Pipeline : public ObjectHandle
	{
		PipelineType type = PipelineType::UNDEFINED;
	};

	enum class QueueType
	{
		GRAPHICS,
		COMPUTE,
		TRANSFER,
	};

	// queueType will be GRAPHICS_QUEUE if begin_command_buffer() was called without parameters
	struct CommandBuffer : public ObjectHandle
	{
		QueueType queueType;
	};
	
	struct PipelineBarrier
	{
		enum class BarrierType
		{
			MEMORY,
			BUFFER,
			IMAGE
		} type;

		struct MemoryBarrier
		{
			ResourceLayout srcLayout;
			ResourceLayout dstLayout;
		};

		struct BufferBarrier
		{
			Buffer* buffer;
			ResourceLayout srcLayout;
			ResourceLayout dstLayout;
		};

		struct TextureBarrier
		{
			Texture* texture;
			ResourceLayout srcLayout;
			ResourceLayout dstLayout;
			uint32_t baseMipLevel;
			uint32_t baseLayer;
		};

		union
		{
			MemoryBarrier memoryBarrier;
			BufferBarrier bufferBarrier;
			TextureBarrier textureBarrier;
		};

		static PipelineBarrier set_memory_barrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
		{
			MemoryBarrier memoryBarrier;
			memoryBarrier.srcLayout = srcLayout;
			memoryBarrier.dstLayout = dstLayout;
			PipelineBarrier pipelineBarrier;
			pipelineBarrier.type = BarrierType::MEMORY;
			pipelineBarrier.memoryBarrier = memoryBarrier;
			return pipelineBarrier;
		}

		static PipelineBarrier set_buffer_barrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
		{
			BufferBarrier bufferBarrier;
			bufferBarrier.buffer = buffer;
			bufferBarrier.srcLayout = srcLayout;
			bufferBarrier.dstLayout = dstLayout;
			PipelineBarrier pipelineBarrier;
			pipelineBarrier.bufferBarrier = bufferBarrier;
			pipelineBarrier.type = BarrierType::BUFFER;
			return pipelineBarrier;
		}

		static PipelineBarrier set_texture_barrier(
			Texture* texture,
			ResourceLayout srcLayout,
			ResourceLayout dstLayout,
			uint32_t baseMipLevel = 0,
			uint32_t baseLayer = 0)
		{
			TextureBarrier textureBarrier;
			textureBarrier.texture = texture;
			textureBarrier.srcLayout = srcLayout;
			textureBarrier.dstLayout = dstLayout;
			textureBarrier.baseMipLevel = baseMipLevel;
			textureBarrier.baseLayer = baseLayer;
			PipelineBarrier pipelineBarrier;
			pipelineBarrier.textureBarrier = textureBarrier;
			pipelineBarrier.type = BarrierType::IMAGE;
			return pipelineBarrier;
		}
	};
}

template<>
struct EnableBitMaskOperator<ad_astris::rhi::ResourceUsage>
{
	static const bool enable = true;
};

template<>
struct EnableBitMaskOperator<ad_astris::rhi::ResourceFlags>
{
	static const bool enable = true;
};

template<>
struct EnableBitMaskOperator<ad_astris::rhi::ResourceLayout>
{
	static const bool enable = true;
};
