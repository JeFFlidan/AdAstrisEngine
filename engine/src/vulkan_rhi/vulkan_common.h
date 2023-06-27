#pragma once

#include "api.h"
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
	
	VkFormat VK_RHI_API get_format(rhi::Format format);
	VkSampleCountFlagBits VK_RHI_API get_sample_count(rhi::SampleCount sampleCount);
	VkImageType VK_RHI_API get_image_type(rhi::TextureDimension dimension);
	VmaMemoryUsage VK_RHI_API get_memory_usage(rhi::MemoryUsage memoryUsage);
	void VK_RHI_API get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo);
	VkBorderColor VK_RHI_API get_border_color(rhi::BorderColor borderColor);
	VkSamplerAddressMode VK_RHI_API get_address_mode(rhi::AddressMode addressMode);
	VkBufferUsageFlags VK_RHI_API get_buffer_usage(rhi::ResourceUsage usage);
	VkImageUsageFlags VK_RHI_API get_image_usage(rhi::ResourceUsage usage);
	VkPrimitiveTopology VK_RHI_API get_primitive_topology(rhi::TopologyType topologyType);
	VkPolygonMode VK_RHI_API get_polygon_mode(rhi::PolygonMode polygonMode);
	VkCullModeFlags VK_RHI_API get_cull_mode(rhi::CullMode cullMode);
	VkFrontFace VK_RHI_API get_front_face(rhi::FrontFace frontFace);
	VkShaderStageFlags VK_RHI_API get_shader_stage(rhi::ShaderType shaderType);
	VkLogicOp VK_RHI_API get_logic_op(rhi::LogicOp logicOp);
	VkBlendFactor VK_RHI_API get_blend_factor(rhi::BlendFactor blendFactor);
	VkBlendOp VK_RHI_API get_blend_op(rhi::BlendOp blendOp);
	VkCompareOp VK_RHI_API get_compare_op(rhi::CompareOp compareOp);
	VkStencilOp VK_RHI_API get_stencil_op(rhi::StencilOp stencilOp);
	VkAttachmentLoadOp VK_RHI_API get_attach_load_op(rhi::LoadOp loadOp);
	VkAttachmentStoreOp VK_RHI_API get_attach_store_op(rhi::StoreOp storeOp);
	VkImageLayout VK_RHI_API get_image_layout(rhi::ResourceLayout resourceLayout);
	VkAccessFlags VK_RHI_API get_access(rhi::ResourceLayout resourceLayout);
	VkPipelineBindPoint VK_RHI_API get_pipeline_bind_point(rhi::PipelineType pipelineType);
	VkImageAspectFlags VK_RHI_API get_image_aspect(rhi::ResourceUsage usage);

	inline VulkanCommandBuffer* VK_RHI_API get_vk_obj(rhi::CommandBuffer* cmd)
	{
		return static_cast<VulkanCommandBuffer*>(cmd->handle);
	}

	inline VulkanRenderPass* VK_RHI_API get_vk_obj(rhi::RenderPass* pass)
	{
		return static_cast<VulkanRenderPass*>(pass->handle);
	}
	
	inline VulkanPipeline* VK_RHI_API get_vk_obj(rhi::Pipeline* pipeline)
	{
		return static_cast<VulkanPipeline*>(pipeline->handle);
	}

	inline VulkanBuffer* VK_RHI_API get_vk_obj(rhi::Buffer* buffer)
	{
		return static_cast<VulkanBuffer*>(buffer->data);
	}
	
	inline VulkanTexture* VK_RHI_API get_vk_obj(rhi::Texture* texture)
	{
		return static_cast<VulkanTexture*>(texture->data);
	}
	
	inline VkImageView VK_RHI_API get_vk_obj(rhi::TextureView* view)
	{
		return *static_cast<VkImageView*>(view->handle);
	}
	
	inline VkSampler VK_RHI_API get_vk_obj(rhi::Sampler* sampler)
	{
		return *static_cast<VkSampler*>(sampler->handle);
	}
}
