﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <glm/mat4x4.hpp>
#include "vk_mem_alloc.h"

#include <vector>

class VulkanEngine;

class AllocatedBuffer
{
	public:
		VkBuffer _buffer = VK_NULL_HANDLE;
		VmaAllocation _allocation;

		size_t _bufferSize = 0;

		AllocatedBuffer() = default;
		AllocatedBuffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void create_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		
		void copy_from(VulkanEngine* engine, void* srcBuffer, size_t sizeInBytes);
		void destroy_buffer(VulkanEngine* engine);
		VkDescriptorBufferInfo get_info(bool isStorage = false, VkDeviceSize offset = 0);
		static void copy_buffer_cmd(VulkanEngine* engine,
			VkCommandBuffer cmd,
			AllocatedBuffer* srcBuffer,
			AllocatedBuffer* dstBuffer,
			VkDeviceSize dstOffset = 0, 
			VkDeviceSize srcOffset = 0);
		protected:
			void allocate_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);		
};

// Typed version of AllocatedBuffer
template<typename T>
class AllocatedBufferT : public AllocatedBuffer
{
	public:
		AllocatedBufferT<T>() = default;
		AllocatedBufferT<T>(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
		{
			allocate_buffer(engine, allocSize, usage, memoryUsage);
		}
		
		static void copy_typed_buffer_cmd(VulkanEngine* engine, VkCommandBuffer cmd, AllocatedBufferT<T>* srcBuffer, AllocatedBufferT<T>* dstBuffer, VkDeviceSize dstOffset = 0, VkDeviceSize srcOffset = 0)
		{
			VkBufferCopy copy;
			copy.dstOffset = dstOffset;
			copy.srcOffset = srcOffset;
			copy.size = srcBuffer->_bufferSize;
			vkCmdCopyBuffer(cmd, srcBuffer->_buffer, dstBuffer->_buffer, 1, &copy);
		}

	private:
		using AllocatedBuffer::copy_buffer_cmd;
};

struct AllocatedImage
{
	VkImage image;
	VmaAllocation allocation;
	int mipLevels;
};

struct Texture
{
	AllocatedImage imageData;
	VkImageView imageView;

	void destroy_texture(VulkanEngine* engine);
};

struct Attachment : public Texture
{
	VkFormat format;
	VkExtent3D extent;

	void destroy_attachment(VulkanEngine* engine);
};

struct DirShadowMap
{
	// Struct for directional light's shadow
	Attachment attachment;
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	AllocatedBuffer dirLightBuffer;		// It's an uniform buffer to bake shadow maps

	void destroy_shadow_map(VulkanEngine* engine);
};

struct PointShadowMap
{
	Attachment attachment;
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	AllocatedBuffer pointLightBuffer;

	glm::mat4 lightProjMat;
	glm::mat4 lightViewMat;

	void destroy_shadow_map(VulkanEngine* engine);
};

