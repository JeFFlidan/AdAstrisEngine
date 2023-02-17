﻿#include "vulkan_rhi.h"

#include "profiler/logger.h"

using namespace ad_astris;

void vulkan::VulkanRHI::init()
{
	
}

void vulkan::VulkanRHI::cleanup()
{
	
}

void vulkan::VulkanRHI::create_buffer(rhi::BufferInfo* info, void* data, uint64_t size)
{
	
}

void vulkan::VulkanRHI::create_texture(rhi::TextureInfo* info, void* data, uint64_t size)
{
	
}

void vulkan::VulkanRHI::create_sampler(rhi::SamplerInfo* info)
{
	
}

void vulkan::VulkanRHI::update_buffer_data(rhi::BufferInfo* info, void* data, uint64_t size)
{
	
}

// private methods
VkFormat vulkan::VulkanRHI::get_texture_format(rhi::TextureFormat format)
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

VmaMemoryUsage vulkan::VulkanRHI::get_memory_usage(rhi::MemoryUsage memoryUsage)
{
	switch (memoryUsage)
	{
		case rhi::CPU:
			return VMA_MEMORY_USAGE_CPU_ONLY;
		case rhi::GPU:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		case rhi::CPU_TO_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
}

void vulkan::VulkanRHI::get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo)
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

VkBorderColor vulkan::VulkanRHI::get_border_color(rhi::BorderColor borderColor)
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

VkSamplerAddressMode vulkan::VulkanRHI::get_address_mode(rhi::AddressMode addressMode)
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

VkBufferUsageFlags vulkan::VulkanRHI::get_buffer_usage(rhi::ResourceUsage usage)
{
	switch (usage)
	{
		case rhi::SAMPLED:
		case rhi::STORAGE:
		case rhi::COLOR_ATTACHMENT:
		case rhi::DEPTH_STENCIL_ATTACHMENT:
		case rhi::TRANSIENT_ATTACHMENT:
		case rhi::INPUT_ATTACHMENT:
		case rhi::UNDEFINED_USAGE:
			LOG_ERROR("Invalid usage. Can't use buffer. VK_BUFFER_USAGE_INDEX_BUFFER will be returned")
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
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

VkImageUsageFlags vulkan::VulkanRHI::get_image_usage(rhi::ResourceUsage usage)
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
			LOG_ERROR("Invalid usage. Can't use image. VK_IMAGE_USAGE_STORAGE_BIT will be returned")
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case rhi::SAMPLED:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case rhi::STORAGE:
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
