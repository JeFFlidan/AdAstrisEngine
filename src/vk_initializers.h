// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vulkan/vulkan_core.h>

namespace vkinit 
{
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, 
		uint32_t count = 1, 
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkPipelineDynamicStateCreateInfo dynamic_state_create_info(VkDynamicState* dynamicStates, uint32_t count, VkPipelineDynamicStateCreateFlags flags = 0);

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderPass, VkExtent2D extent, VkFramebuffer framebuffer);

	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);
	VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
	VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);
	VkSubmitInfo submit_info(VkCommandBuffer* cmd);

	VkImageMemoryBarrier image_barrier(VkImage image, VkAccessFlags srcMask, VkAccessFlags dstMask, VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageAspectFlags aspectFlag);
	VkBufferMemoryBarrier buffer_barrier(AllocatedBuffer* buffer, VkAccessFlags srcMask, VkAccessFlags dstMask, uint32_t queueFamily, VkDeviceSize offset = 0);

	VkFramebufferCreateInfo framebuffer_create_info(VkImageView* imageViews, uint32_t count, VkRenderPass renderPass, VkExtent3D extent, VkFramebufferCreateFlags flags = 0);
}

