#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace engine::vulkan
{
	class VulkanRHI : public rhi::IEngineRHI
	{
		public:
			~VulkanRHI() final override = default;
		
			virtual void init() final override;
			virtual void cleanup() final override;

			virtual void create_buffer(rhi::BufferInfo* info, void* data = nullptr, uint64_t size = 0) final override;
			virtual void create_texture(rhi::TextureInfo* info, void* data = nullptr, uint64_t size = 0) final override;
			virtual void create_sampler(rhi::SamplerInfo* info) final override;

			virtual void update_buffer_data(rhi::BufferInfo* info, void* data, uint64_t size) final override;

		private:
			VkInstance _instance;
			VmaAllocator _allocator;
		
			VkFormat get_texture_format(rhi::TextureFormat format);
			VmaMemoryUsage get_memory_usage(rhi::MemoryUsage memoryUsage);
			void get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo);
			VkBorderColor get_border_color(rhi::BorderColor borderColor);
			VkSamplerAddressMode get_address_mode(rhi::AddressMode addressMode);
			VkBufferUsageFlags get_buffer_usage(rhi::ResourceUsage usage);
			VkImageUsageFlags get_image_usage(rhi::ResourceUsage usage);
	};
}
