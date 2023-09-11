#include "core/flags_operations.h"
#include "vulkan_command_manager.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;

void vulkan::create_semaphore(VkDevice device, VkSemaphore* semaphore)
{
	VkSemaphoreCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.flags = 0;
	VK_CHECK(vkCreateSemaphore(device, &info, nullptr, semaphore));
}

VkFence vulkan::create_fence(VkDevice device)
{
	VkFenceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	VK_CHECK(vkCreateFence(device, &info, nullptr, &fence));
	return fence;
}

VkFormat vulkan::get_format(rhi::Format format)
{
	switch (format)
	{
		case rhi::Format::UNDEFINED:
			return VK_FORMAT_UNDEFINED;
		case rhi::Format::R4G4_UNORM:
			return VK_FORMAT_R4G4_UNORM_PACK8;
		case rhi::Format::R4G4B4A4_UNORM:
			return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		case rhi::Format::B4G4R4A4_UNORM:
			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case rhi::Format::R5G5B5A1_UNORM:
			return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		case rhi::Format::B5G5R5A1_UNORM:
			return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case rhi::Format::A1R5G5B5_UNORM:
			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

		case rhi::Format::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case rhi::Format::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case rhi::Format::R8_UINT:
			return VK_FORMAT_R8_UINT;
		case rhi::Format::R8_SINT:
			return VK_FORMAT_R8_SINT;
		case rhi::Format::R8_SRGB:
			return VK_FORMAT_R8_SRGB;

		case rhi::Format::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case rhi::Format::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case rhi::Format::R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case rhi::Format::R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case rhi::Format::R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;

		case rhi::Format::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case rhi::Format::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case rhi::Format::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case rhi::Format::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case rhi::Format::R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;

		case rhi::Format::B8G8R8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case rhi::Format::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case rhi::Format::B8G8R8A8_SNORM:
			return VK_FORMAT_B8G8R8A8_SNORM;

		case rhi::Format::R10G10B10A2_UNORM:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case rhi::Format::R10G10B10A2_SNORM:
			return VK_FORMAT_A2R10G10B10_SNORM_PACK32;

		case rhi::Format::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case rhi::Format::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case rhi::Format::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case rhi::Format::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case rhi::Format::R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;

		case rhi::Format::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case rhi::Format::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case rhi::Format::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case rhi::Format::R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case rhi::Format::R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;

		case rhi::Format::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case rhi::Format::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case rhi::Format::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case rhi::Format::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case rhi::Format::R16G16B16A16_SFLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;

		case rhi::Format::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case rhi::Format::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case rhi::Format::R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case rhi::Format::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case rhi::Format::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case rhi::Format::R32G32_SFLOAT:
			return VK_FORMAT_R32G32_SFLOAT;

		case rhi::Format::R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case rhi::Format::R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case rhi::Format::R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;

		case rhi::Format::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case rhi::Format::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case rhi::Format::R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;

		case rhi::Format::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case rhi::Format::D32_SFLOAT:
			return VK_FORMAT_D32_SFLOAT;

		case rhi::Format::S8_UINT:
			return VK_FORMAT_S8_UINT;
		case rhi::Format::D16_UNORM_S8_UINT:
			return VK_FORMAT_D16_UNORM_S8_UINT;
		case rhi::Format::D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case rhi::Format::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits vulkan::get_sample_count(rhi::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case rhi::SampleCount::BIT_1:
			return VK_SAMPLE_COUNT_1_BIT;
		case rhi::SampleCount::BIT_2:
			return VK_SAMPLE_COUNT_2_BIT;
		case rhi::SampleCount::BIT_4:
			return VK_SAMPLE_COUNT_4_BIT;
		case rhi::SampleCount::BIT_8:
			return VK_SAMPLE_COUNT_8_BIT;
		case rhi::SampleCount::BIT_16:
			return VK_SAMPLE_COUNT_16_BIT;
		case rhi::SampleCount::BIT_32:
			return VK_SAMPLE_COUNT_32_BIT;
		case rhi::SampleCount::BIT_64:
			return VK_SAMPLE_COUNT_64_BIT;
	}
}

VmaMemoryUsage vulkan::get_memory_usage(rhi::MemoryUsage memoryUsage)
{
	switch (memoryUsage)
	{
		case rhi::MemoryUsage::UNDEFINED:
			LOG_ERROR("Undefined memory usage")
			return VMA_MEMORY_USAGE_UNKNOWN;
		case rhi::MemoryUsage::CPU:
			return VMA_MEMORY_USAGE_CPU_ONLY;
		case rhi::MemoryUsage::GPU:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		case rhi::MemoryUsage::CPU_TO_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
}

VkImageType vulkan::get_image_type(rhi::TextureDimension dimension)
{
	switch (dimension)
	{
		case rhi::TextureDimension::TEXTURE1D:
			return VK_IMAGE_TYPE_1D;
		case rhi::TextureDimension::TEXTURE2D:
			return VK_IMAGE_TYPE_2D;
		case rhi::TextureDimension::TEXTURE3D:
			return VK_IMAGE_TYPE_3D;
	}
}

void vulkan::get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo)
{
	switch (filter)
	{
		case rhi::Filter::MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::COMPARISON_MIN_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::Filter::MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::Filter::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::Filter::MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::Filter::MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::Filter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::Filter::MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::Filter::MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::Filter::ANISOTROPIC:
		case rhi::Filter::MINIMUM_ANISOTROPIC:
		case rhi::Filter::MAXIMUM_ANISOTROPIC:
		case rhi::Filter::COMPARISON_ANISOTROPIC:
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
		case rhi::BorderColor::FLOAT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case rhi::BorderColor::INT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case rhi::BorderColor::FLOAT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case rhi::BorderColor::INT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case rhi::BorderColor::FLOAT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case rhi::BorderColor::INT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
}

VkSamplerAddressMode vulkan::get_address_mode(rhi::AddressMode addressMode)
{
	switch (addressMode)
	{
		case rhi::AddressMode::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case rhi::AddressMode::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case rhi::AddressMode::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case rhi::AddressMode::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case rhi::AddressMode::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}
}

VkBufferUsageFlags vulkan::get_buffer_usage(rhi::ResourceUsage usage)
{
	VkBufferUsageFlags usageFlags = 0;
	if (has_flag(usage, rhi::ResourceUsage::TRANSFER_SRC))
	{
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::TRANSFER_DST))
	{
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::STORAGE_TEXEL_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::UNIFORM_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::STORAGE_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::INDEX_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::VERTEX_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::INDIRECT_BUFFER))
	{
		usageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
	return usageFlags;
}

VkImageUsageFlags vulkan::get_image_usage(rhi::ResourceUsage usage)
{
	VkImageUsageFlags usageFlags = 0;
	if (has_flag(usage, rhi::ResourceUsage::TRANSFER_SRC))
	{
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::TRANSFER_DST))
	{
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::SAMPLED_TEXTURE))
	{
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::STORAGE_TEXTURE))
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::COLOR_ATTACHMENT))
	{
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
	{
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::TRANSIENT_ATTACHMENT))
	{
		usageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}
	if (has_flag(usage, rhi::ResourceUsage::INPUT_ATTACHMENT))
	{
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	return usageFlags;
}

VkPrimitiveTopology vulkan::get_primitive_topology(rhi::TopologyType topologyType)
{
	switch (topologyType)
	{
		case rhi::TopologyType::POINT:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case rhi::TopologyType::LINE:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case rhi::TopologyType::TRIANGLE:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case rhi::TopologyType::PATCH:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}
}

VkPolygonMode vulkan::get_polygon_mode(rhi::PolygonMode polygonMode)
{
	switch (polygonMode)
	{
		case rhi::PolygonMode::FILL:
			return VK_POLYGON_MODE_FILL;
		case rhi::PolygonMode::LINE:
			return VK_POLYGON_MODE_LINE;
		case rhi::PolygonMode::POINT:
			return VK_POLYGON_MODE_POINT;
	}
}

VkCullModeFlags vulkan::get_cull_mode(rhi::CullMode cullMode)
{
	switch (cullMode)
	{
		case rhi::CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case rhi::CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case rhi::CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		case rhi::CullMode::FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
	}
}

VkFrontFace vulkan::get_front_face(rhi::FrontFace frontFace)
{
	switch (frontFace)
	{
		case rhi::FrontFace::CLOCKWISE:
			return VK_FRONT_FACE_CLOCKWISE;
		case rhi::FrontFace::COUNTER_CLOCKWISE:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

VkShaderStageFlags vulkan::get_shader_stage(rhi::ShaderType shaderType)
{
	switch (shaderType)
	{
		case rhi::ShaderType::UNDEFINED:
			LOG_ERROR("Undefined shader type")
			return 0;
		case rhi::ShaderType::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case rhi::ShaderType::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case rhi::ShaderType::TESSELLATION_CONTROL:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case rhi::ShaderType::TESSELLATION_EVALUATION:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case rhi::ShaderType::GEOMETRY:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case rhi::ShaderType::COMPUTE:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case rhi::ShaderType::MESH:
			return VK_SHADER_STAGE_MESH_BIT_NV;
		case rhi::ShaderType::TASK:
			return VK_SHADER_STAGE_TASK_BIT_NV;
		case rhi::ShaderType::RAY_GENERATION:
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case rhi::ShaderType::RAY_INTERSECTION:
			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case rhi::ShaderType::RAY_ANY_HIT:
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case rhi::ShaderType::RAY_CLOSEST_HIT:
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case rhi::ShaderType::RAY_MISS:
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		case rhi::ShaderType::RAY_CALLABLE:
			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	}
}

VkLogicOp vulkan::get_logic_op(rhi::LogicOp logicOp)
{
	switch (logicOp)
	{
		case rhi::LogicOp::CLEAR:
			return VK_LOGIC_OP_CLEAR;
		case rhi::LogicOp::AND:
			return VK_LOGIC_OP_AND;
		case rhi::LogicOp::AND_REVERSE:
			return VK_LOGIC_OP_AND_REVERSE;
		case rhi::LogicOp::COPY:
			return VK_LOGIC_OP_COPY;
		case rhi::LogicOp::AND_INVERTED:
			return VK_LOGIC_OP_AND_INVERTED;
		case rhi::LogicOp::NO_OP:
			return VK_LOGIC_OP_NO_OP;
		case rhi::LogicOp::XOR:
			return VK_LOGIC_OP_XOR;
		case rhi::LogicOp::OR:
			return VK_LOGIC_OP_OR;
		case rhi::LogicOp::NOR:
			return VK_LOGIC_OP_NOR;
		case rhi::LogicOp::EQUIVALENT:
			return VK_LOGIC_OP_EQUIVALENT;
		case rhi::LogicOp::INVERT:
			return VK_LOGIC_OP_INVERT;
		case rhi::LogicOp::OR_REVERSE:
			return VK_LOGIC_OP_OR_REVERSE;
		case rhi::LogicOp::COPY_INVERTED:
			return VK_LOGIC_OP_COPY_INVERTED;
		case rhi::LogicOp::OR_INVERTED:
			return VK_LOGIC_OP_OR_INVERTED;
		case rhi::LogicOp::NAND:
			return VK_LOGIC_OP_NAND;
		case rhi::LogicOp::SET:
			return VK_LOGIC_OP_SET;
	}
}

VkBlendFactor vulkan::get_blend_factor(rhi::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case rhi::BlendFactor::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case rhi::BlendFactor::ONE:
			return VK_BLEND_FACTOR_ONE;
		case rhi::BlendFactor::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case rhi::BlendFactor::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case rhi::BlendFactor::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case rhi::BlendFactor::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case rhi::BlendFactor::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case rhi::BlendFactor::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case rhi::BlendFactor::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case rhi::BlendFactor::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case rhi::BlendFactor::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case rhi::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case rhi::BlendFactor::CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case rhi::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		case rhi::BlendFactor::SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case rhi::BlendFactor::SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case rhi::BlendFactor::ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case rhi::BlendFactor::SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case rhi::BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
}

VkBlendOp vulkan::get_blend_op(rhi::BlendOp blendOp)
{
	switch (blendOp)
	{
		case rhi::BlendOp::ADD:
			return VK_BLEND_OP_ADD;
		case rhi::BlendOp::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case rhi::BlendOp::REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case rhi::BlendOp::MIN:
			return VK_BLEND_OP_MIN;
		case rhi::BlendOp::MAX:
			return VK_BLEND_OP_MAX;
	}
}

VkCompareOp vulkan::get_compare_op(rhi::CompareOp compareOp)
{
	switch (compareOp)
	{
		case rhi::CompareOp::NEVER:
			return VK_COMPARE_OP_NEVER;
		case rhi::CompareOp::LESS:
			return VK_COMPARE_OP_LESS;
		case rhi::CompareOp::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case rhi::CompareOp::LESS_OR_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case rhi::CompareOp::GREATER:
			return VK_COMPARE_OP_GREATER;
		case rhi::CompareOp::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case rhi::CompareOp::GREATER_OR_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case rhi::CompareOp::ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
	}
}
VkStencilOp vulkan::get_stencil_op(rhi::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case rhi::StencilOp::KEEP:
			return VK_STENCIL_OP_KEEP;
		case rhi::StencilOp::ZERO:
			return VK_STENCIL_OP_ZERO;
		case rhi::StencilOp::REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case rhi::StencilOp::INCREMENT_AND_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case rhi::StencilOp::DECREMENT_AND_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case rhi::StencilOp::INVERT:
			return VK_STENCIL_OP_INVERT;
		case rhi::StencilOp::INCREMENT_AND_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case rhi::StencilOp::DECREMENT_AND_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
}

VkAttachmentLoadOp vulkan::get_attach_load_op(rhi::LoadOp loadOp)
{
	switch (loadOp)
	{
		case rhi::LoadOp::LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case rhi::LoadOp::CLEAR:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case rhi::LoadOp::DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp vulkan::get_attach_store_op(rhi::StoreOp storeOp)
{
	switch (storeOp)
	{
		case rhi::StoreOp::STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case rhi::StoreOp::DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

VkImageLayout vulkan::get_image_layout(rhi::ResourceLayout resourceLayout)
{
	if (has_flag(resourceLayout, rhi::ResourceLayout::UNDEFINED))
	{
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::GENERAL))
	{
		return VK_IMAGE_LAYOUT_GENERAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_READ))
	{
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::COLOR_ATTACHMENT))
	{
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL))
	{
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL_READ_ONLY))
	{
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_SRC))
	{
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_DST))
	{
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::PRESENT_ATTACHMENT))
	{
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
}

VkAccessFlags vulkan::get_access(rhi::ResourceLayout resourceLayout)
{
	VkAccessFlags accessFlags = 0;
	if (has_flag(resourceLayout, rhi::ResourceLayout::INDIRECT_COMMAND_BUFFER))
	{
		accessFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::INDEX_BUFFER))
	{
		accessFlags |= VK_ACCESS_INDEX_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::VERTEX_BUFFER))
	{
		accessFlags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::UNIFORM_BUFFER))
	{
		accessFlags |= VK_ACCESS_UNIFORM_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_READ))
	{
		accessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_WRITE))
	{
		accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::COLOR_ATTACHMENT))
	{
		accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL))
	{
		accessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL_READ_ONLY))
	{
		accessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_SRC))
	{
		accessFlags |= VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_DST))
	{
		accessFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::MEMORY_READ))
	{
		accessFlags |= VK_ACCESS_MEMORY_READ_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::MEMORY_WRITE))
	{
		accessFlags |= VK_ACCESS_MEMORY_WRITE_BIT;
	}
	if (has_flag(resourceLayout, rhi::ResourceLayout::GENERAL))
	{
		accessFlags |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	}
	return accessFlags;
}

VkPipelineBindPoint vulkan::get_pipeline_bind_point(rhi::PipelineType pipelineType)
{
	switch (pipelineType)
	{
		case rhi::PipelineType::GRAPHICS:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case rhi::PipelineType::RAY_TRACING:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	}
}

VkImageAspectFlags vulkan::get_image_aspect(rhi::ResourceUsage usage)
{
	if (has_flag(usage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	return VK_IMAGE_ASPECT_COLOR_BIT;
}
