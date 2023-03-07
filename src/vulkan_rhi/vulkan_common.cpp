#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;

VkFormat vulkan::get_format(rhi::Format format)
{
	switch (format)
	{
		case rhi::UNDEFINED_FORMAT:
			return VK_FORMAT_UNDEFINED;
		case rhi::R4G4_UNORM:
			return VK_FORMAT_R4G4_UNORM_PACK8;
		case rhi::R4G4B4A4_UNORM:
			return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		case rhi::B4G4R4A4_UNORM:
			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case rhi::R5G5B5A1_UNORM:
			return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		case rhi::B5G5R5A1_UNORM:
			return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case rhi::A1R5G5B5_UNORM:
			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			
		case rhi::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case rhi::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case rhi::R8_UINT:
			return VK_FORMAT_R8_UINT;
		case rhi::R8_SINT:
			return VK_FORMAT_R8_SINT;
		case rhi::R8_SRGB:
			return VK_FORMAT_R8_SRGB;
			
		case rhi::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case rhi::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case rhi::R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case rhi::R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case rhi::R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;

		case rhi::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case rhi::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case rhi::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case rhi::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case rhi::R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;

		case rhi::B8G8R8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case rhi::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case rhi::B8G8R8A8_SNORM:
			return VK_FORMAT_B8G8R8A8_SNORM;

		case rhi::R10G10B10A2_UNORM:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case rhi::R10G10B10A2_SNORM:
			return VK_FORMAT_A2R10G10B10_SNORM_PACK32;

		case rhi::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case rhi::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case rhi::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case rhi::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case rhi::R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;

		case rhi::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case rhi::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case rhi::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case rhi::R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case rhi::R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;

		case rhi::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case rhi::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case rhi::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case rhi::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case rhi::R16G16B16A16_SFLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;

		case rhi::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case rhi::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case rhi::R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case rhi::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case rhi::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case rhi::R32G32_SFLOAT:
			return VK_FORMAT_R32G32_SFLOAT;

		case rhi::R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case rhi::R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case rhi::R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;

		case rhi::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case rhi::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case rhi::R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;

		case rhi::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case rhi::D32_SFLOAT:
			return VK_FORMAT_D32_SFLOAT;

		case rhi::S8_UINT:
			return VK_FORMAT_S8_UINT;
		case rhi::D16_UNORM_S8_UINT:
			return VK_FORMAT_D16_UNORM_S8_UINT;
		case rhi::D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case rhi::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits vulkan::get_sample_count(rhi::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case rhi::SAMPLE_COUNT_1_BIT:
			return VK_SAMPLE_COUNT_1_BIT;
		case rhi::SAMPLE_COUNT_2_BIT:
			return VK_SAMPLE_COUNT_2_BIT;
		case rhi::SAMPLE_COUNT_4_BIT:
			return VK_SAMPLE_COUNT_4_BIT;
		case rhi::SAMPLE_COUNT_8_BIT:
			return VK_SAMPLE_COUNT_8_BIT;
		case rhi::SAMPLE_COUNT_16_BIT:
			return VK_SAMPLE_COUNT_16_BIT;
		case rhi::SAMPLE_COUNT_32_BIT:
			return VK_SAMPLE_COUNT_32_BIT;
		case rhi::SAMPLE_COUNT_64_BIT:
			return VK_SAMPLE_COUNT_64_BIT;
	}
}

VmaMemoryUsage vulkan::get_memory_usage(rhi::MemoryUsage memoryUsage)
{
	switch (memoryUsage)
	{
		case rhi::UNDEFINED_MEMORY_USAGE:
			LOG_ERROR("Undefined memory usage")
			return VMA_MEMORY_USAGE_UNKNOWN;
		case rhi::CPU:
			return VMA_MEMORY_USAGE_CPU_ONLY;
		case rhi::GPU:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		case rhi::CPU_TO_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
}

VkImageType vulkan::get_image_type(rhi::TextureDimension dimension)
{
	switch (dimension)
	{
		case rhi::TEXTURE1D:
			return VK_IMAGE_TYPE_1D;
		case rhi::TEXTURE2D:
			return VK_IMAGE_TYPE_2D;
		case rhi::TEXTURE3D:
			return VK_IMAGE_TYPE_3D;
	}
}

void vulkan::get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo)
{
	switch (filter)
	{
		case rhi::MIN_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::COMPARISON_MIN_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_MAG_MIP_LINEAR:
		case rhi::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::COMPARISON_MIN_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::ANISOTROPIC:
		case rhi::MINIMUM_ANISOTROPIC:
		case rhi::MAXIMUM_ANISOTROPIC:
		case rhi::COMPARISON_ANISOTROPIC:
			// have to think about this
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.anisotropyEnable = VK_TRUE;
			break;
		default:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
	}
}

VkBorderColor vulkan::get_border_color(rhi::BorderColor borderColor)
{
	switch (borderColor)
	{
		case rhi::FLOAT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case rhi::INT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case rhi::FLOAT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case rhi::INT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case rhi::FLOAT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case rhi::INT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
}

VkSamplerAddressMode vulkan::get_address_mode(rhi::AddressMode addressMode)
{
	switch (addressMode)
	{
		case rhi::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case rhi::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case rhi::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case rhi::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case rhi::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}
}

VkBufferUsageFlags vulkan::get_buffer_usage(rhi::ResourceUsage usage)
{
	switch (usage)
	{
		case rhi::SAMPLED_TEXTURE:
		case rhi::STORAGE_TEXTURE:
		case rhi::COLOR_ATTACHMENT:
		case rhi::DEPTH_STENCIL_ATTACHMENT:
		case rhi::TRANSIENT_ATTACHMENT:
		case rhi::INPUT_ATTACHMENT:
		case rhi::UNDEFINED_USAGE:
			LOG_ERROR("Invalid usage. Can't use buffer. 0 will be returned")
			return 0;
		case rhi::UNIFORM_TEXEL_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		case rhi::STORAGE_TEXEL_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		case rhi::UNIFORM_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case rhi::STORAGE_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case rhi::INDEX_BUFFER:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case rhi::VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case rhi::INDIRECT_BUFFER:
			return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
}

VkImageUsageFlags vulkan::get_image_usage(rhi::ResourceUsage usage)
{
	switch (usage)
	{
		case rhi::UNIFORM_TEXEL_BUFFER:
		case rhi::STORAGE_TEXEL_BUFFER:
		case rhi::UNIFORM_BUFFER:
		case rhi::STORAGE_BUFFER:
		case rhi::INDEX_BUFFER:
		case rhi::VERTEX_BUFFER:
		case rhi::INDIRECT_BUFFER:
		case rhi::UNDEFINED_USAGE:
			LOG_ERROR("Invalid usage. Can't use image. 0 will be returned")
			return 0;
		case rhi::SAMPLED_TEXTURE:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case rhi::STORAGE_TEXTURE:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case rhi::COLOR_ATTACHMENT:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case rhi::DEPTH_STENCIL_ATTACHMENT:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case rhi::TRANSIENT_ATTACHMENT:
			return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		case rhi::INPUT_ATTACHMENT:
			return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
}

VkPrimitiveTopology vulkan::get_primitive_topology(rhi::TopologyType topologyType)
{
	switch (topologyType)
	{
		case rhi::TOPOLOGY_POINT:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case rhi::TOPOLOGY_LINE:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case rhi::TOPOLOGY_TRIANGLE:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case rhi::TOPOLOGY_PATCH:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}
}

VkPolygonMode vulkan::get_polygon_mode(rhi::PolygonMode polygonMode)
{
	switch (polygonMode)
	{
		case rhi::POLYGON_MODE_FILL:
			return VK_POLYGON_MODE_FILL;
		case rhi::POLYGON_MODE_LINE:
			return VK_POLYGON_MODE_LINE;
		case rhi::POLYGON_MODE_POINT:
			return VK_POLYGON_MODE_POINT;
	}
}

VkCullModeFlags vulkan::get_cull_mode(rhi::CullMode cullMode)
{
	switch (cullMode)
	{
		case rhi::CULL_MODE_NONE:
			return VK_CULL_MODE_NONE;
		case rhi::CULL_MODE_FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case rhi::CULL_MODE_BACK:
			return VK_CULL_MODE_BACK_BIT;
		case rhi::CULL_MODE_FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
	}
}

VkFrontFace vulkan::get_front_face(rhi::FrontFace frontFace)
{
	switch (frontFace)
	{
		case rhi::FRONT_FACE_CLOCKWISE:
			return VK_FRONT_FACE_CLOCKWISE;
		case rhi::FRONT_FACE_COUNTER_CLOCKWISE:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

VkShaderStageFlags vulkan::get_shader_stage(rhi::ShaderType shaderType)
{
	switch (shaderType)
	{
		case rhi::UNDEFINED_SHADER_TYPE:
			LOG_ERROR("Undefined shader type")
			return 0;
		case rhi::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case rhi::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case rhi::TESSELLATION_CONTROL:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case rhi::TESSELLATION_EVALUATION:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case rhi::GEOMETRY:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case rhi::COMPUTE:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case rhi::MESH:
			return VK_SHADER_STAGE_MESH_BIT_NV;
		case rhi::TASK:
			return VK_SHADER_STAGE_TASK_BIT_NV;
		case rhi::RAY_GENERATION:
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case rhi::RAY_INTERSECTION:
			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case rhi::RAY_ANY_HIT:
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case rhi::RAY_CLOSEST_HIT:
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case rhi::RAY_MISS:
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		case rhi::RAY_CALLABLE:
			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	}
}

VkLogicOp vulkan::get_logic_op(rhi::LogicOp logicOp)
{
	switch (logicOp)
	{
		case rhi::LOGIC_OP_CLEAR:
			return VK_LOGIC_OP_CLEAR;
		case rhi::LOGIC_OP_AND:
			return VK_LOGIC_OP_AND;
		case rhi::LOGIC_OP_AND_REVERSE:
			return VK_LOGIC_OP_AND_REVERSE;
		case rhi::LOGIC_OP_COPY:
			return VK_LOGIC_OP_COPY;
		case rhi::LOGIC_OP_AND_INVERTED:
			return VK_LOGIC_OP_AND_INVERTED;
		case rhi::LOGIC_OP_NO_OP:
			return VK_LOGIC_OP_NO_OP;
		case rhi::LOGIC_OP_XOR:
			return VK_LOGIC_OP_XOR;
		case rhi::LOGIC_OP_OR:
			return VK_LOGIC_OP_OR;
		case rhi::LOGIC_OP_NOR:
			return VK_LOGIC_OP_NOR;
		case rhi::LOGIC_OP_EQUIVALENT:
			return VK_LOGIC_OP_EQUIVALENT;
		case rhi::LOGIC_OP_INVERT:
			return VK_LOGIC_OP_INVERT;
		case rhi::LOGIC_OP_OR_REVERSE:
			return VK_LOGIC_OP_OR_REVERSE;
		case rhi::LOGIC_OP_COPY_INVERTED:
			return VK_LOGIC_OP_COPY_INVERTED;
		case rhi::LOGIC_OP_OR_INVERTED:
			return VK_LOGIC_OP_OR_INVERTED;
		case rhi::LOGIC_OP_NAND:
			return VK_LOGIC_OP_NAND;
		case rhi::LOGIC_OP_SET:
			return VK_LOGIC_OP_SET;
	}
}

VkBlendFactor vulkan::get_blend_factor(rhi::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case rhi::BLEND_FACTOR_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case rhi::BLEND_FACTOR_ONE:
			return VK_BLEND_FACTOR_ONE;
		case rhi::BLEND_FACTOR_SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case rhi::BLEND_FACTOR_DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case rhi::BLEND_FACTOR_SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case rhi::BLEND_FACTOR_DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case rhi::BLEND_FACTOR_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case rhi::BLEND_FACTOR_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		case rhi::BLEND_FACTOR_SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case rhi::BLEND_FACTOR_SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case rhi::BLEND_FACTOR_SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
}

VkBlendOp vulkan::get_blend_op(rhi::BlendOp blendOp)
{
	switch (blendOp)
	{
		case rhi::BLEND_OP_ADD:
			return VK_BLEND_OP_ADD;
		case rhi::BLEND_OP_SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case rhi::BLEND_OP_REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case rhi::BLEND_OP_MIN:
			return VK_BLEND_OP_MIN;
		case rhi::BLEND_OP_MAX:
			return VK_BLEND_OP_MAX;
	}
}

VkCompareOp vulkan::get_compare_op(rhi::CompareOp compareOp)
{
	switch (compareOp)
	{
		case rhi::COMPARE_OP_NEVER:
			return VK_COMPARE_OP_NEVER;
		case rhi::COMPARE_OP_LESS:
			return VK_COMPARE_OP_LESS;
		case rhi::COMPARE_OP_EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case rhi::COMPARE_OP_LESS_OR_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case rhi::COMPARE_OP_GREATER:
			return VK_COMPARE_OP_GREATER;
		case rhi::COMPARE_OP_NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case rhi::COMPARE_OP_GREATER_OR_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case rhi::COMPARE_OP_ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
	}
}
VkStencilOp vulkan::get_stencil_op(rhi::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case rhi::STENCIL_OP_KEEP:
			return VK_STENCIL_OP_KEEP;
		case rhi::STENCIL_OP_ZERO:
			return VK_STENCIL_OP_ZERO;
		case rhi::STENCIL_OP_REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case rhi::STENCIL_OP_INCREMENT_AND_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case rhi::STENCIL_OP_DECREMENT_AND_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case rhi::STENCIL_OP_INVERT:
			return VK_STENCIL_OP_INVERT;
		case rhi::STENCIL_OP_INCREMENT_AND_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case rhi::STENCIL_OP_DECREMENT_AND_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
}

VkAttachmentLoadOp vulkan::get_attach_load_op(rhi::LoadOp loadOp)
{
	switch (loadOp)
	{
		case rhi::LOAD_OP_LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case rhi::LOAD_OP_CLEAR:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case rhi::LOAD_OP_DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp vulkan::get_attach_store_op(rhi::StoreOp storeOp)
{
	switch (storeOp)
	{
		case rhi::STORE_OP_STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case rhi::STORE_OP_DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

VkImageLayout vulkan::get_image_layout(rhi::ResourceLayout resourceLayout)
{
	switch (resourceLayout)
	{
		case rhi::RESOURCE_LAYOUT_UNDEFINED:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case rhi::RESOURCE_LAYOUT_GENERAL:
			return VK_IMAGE_LAYOUT_GENERAL;
		case rhi::RESOURCE_LAYOUT_SHADER_READ_ONLY:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_TRANSFER_SRC:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_TRANSFER_DST:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_COLOR_ATTACHMENT:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_DEPTH_STENCIL:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_DEPTH_STENCIL_READ_ONLY:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
}

VkPipelineBindPoint vulkan::get_pipeline_bind_point(rhi::PipelineType pipelineType)
{
	switch (pipelineType)
	{
		case rhi::GRAPHICS_PIPELINE:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case rhi::RAY_TRACING_PIPELINE:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	}
}
