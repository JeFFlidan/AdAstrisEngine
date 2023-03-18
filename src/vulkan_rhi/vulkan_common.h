#pragma once

#include "rhi/resources.h"
#include "vulkan_command_manager.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"

#include <iostream>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define VK_CHECK(x)												 \
	do																 \
	{																 \
		VkResult err = x;											 \
		if (err)													 \
		{															 \
			std::cout << "Detected Vulkan error " << err << std::endl;\
			abort();												 \
		}															 \
	} while (0)

namespace ad_astris::vulkan
{
	void create_semaphore(VkDevice device, VkSemaphore* semaphore);
	
	VkFormat get_format(rhi::Format format);
	VkSampleCountFlagBits get_sample_count(rhi::SampleCount sampleCount);
	VkImageType get_image_type(rhi::TextureDimension dimension);
	VmaMemoryUsage get_memory_usage(rhi::MemoryUsage memoryUsage);
	void get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo);
	VkBorderColor get_border_color(rhi::BorderColor borderColor);
	VkSamplerAddressMode get_address_mode(rhi::AddressMode addressMode);
	VkBufferUsageFlags get_buffer_usage(rhi::ResourceUsage usage);
	VkImageUsageFlags get_image_usage(rhi::ResourceUsage usage);
	VkPrimitiveTopology get_primitive_topology(rhi::TopologyType topologyType);
	VkPolygonMode get_polygon_mode(rhi::PolygonMode polygonMode);
	VkCullModeFlags get_cull_mode(rhi::CullMode cullMode);
	VkFrontFace get_front_face(rhi::FrontFace frontFace);
	VkShaderStageFlags get_shader_stage(rhi::ShaderType shaderType);
	VkLogicOp get_logic_op(rhi::LogicOp logicOp);
	VkBlendFactor get_blend_factor(rhi::BlendFactor blendFactor);
	VkBlendOp get_blend_op(rhi::BlendOp blendOp);
	VkCompareOp get_compare_op(rhi::CompareOp compareOp);
	VkStencilOp get_stencil_op(rhi::StencilOp stencilOp);
	VkAttachmentLoadOp get_attach_load_op(rhi::LoadOp loadOp);
	VkAttachmentStoreOp get_attach_store_op(rhi::StoreOp storeOp);
	VkImageLayout get_image_layout(rhi::ResourceLayout resourceLayout);
	VkPipelineBindPoint get_pipeline_bind_point(rhi::PipelineType pipelineType);

	inline VulkanCommandBuffer* get_vk_obj(rhi::CommandBuffer* cmd)
	{
		return static_cast<VulkanCommandBuffer*>(cmd->handle);
	}

	inline VulkanRenderPass* get_vk_obj(rhi::RenderPass* pass)
	{
		return static_cast<VulkanRenderPass*>(pass->handle);
	}
	
	inline VulkanPipeline* get_vk_obj(rhi::Pipeline* pipeline)
	{
		return static_cast<VulkanPipeline*>(pipeline->handle);
	}

	inline VulkanBuffer* get_vk_obj(rhi::Buffer* buffer)
	{
		return static_cast<VulkanBuffer*>(buffer->data);
	}
	
	inline VulkanTexture* get_vk_obj(rhi::Texture* texture)
	{
		return static_cast<VulkanTexture*>(texture->data);
	}
	
	inline VkImageView get_vk_obj(rhi::TextureView* view)
	{
		return *static_cast<VkImageView*>(view->handle);
	}
	
	inline VkSampler get_vk_obj(rhi::Sampler* sampler)
	{
		return *static_cast<VkSampler*>(sampler->handle);
	}
}
