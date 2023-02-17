#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include <VkBootstrap.h>

namespace ad_astris::vulkan
{
	class VulkanRHI : public rhi::IEngineRHI
	{
		public:
			~VulkanRHI() final override = default;

			/** Init necessary Vulkan objects.
			 @param window should be valid pointer to the window (SDL or WinApi)
			 */
			virtual void init(void* window) final override;
			virtual void cleanup() final override;

			/** Create a VulkanBuffer and assign the VulkanBuffer to Buffer.data (void*)  
			 @param buffer should be valid pointer to the rhi::Buffer obj.
			 @param size is a buffer size. Should be > 0
			 @param data is a data for buffer. May be nullptr
			 */
			virtual void create_buffer(rhi::Buffer* buffer, uint64_t size, void* data = nullptr) final override;
			/** Create a VulkanBuffer and assign the VulkanBuffer to Buffer.data (void*)  
			 @param buffer should be valid pointer to the rhi::Buffer obj.
			 @param size is a buffer size. Should be > 0
			 @param data is a data for buffer. Should be a pointer to valid data
			 */
			virtual void update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data) final override;
			virtual void create_texture(rhi::Texture* texture, void* data = nullptr, uint64_t size = 0) final override;
			virtual void create_sampler(rhi::SamplerInfo* info) final override;

			// Only for tests. Will be removed in the final implementation
			VkInstance get_instance() { return _instance; }
			VulkanDevice get_device() { return _vulkanDevice; }
			VmaAllocator get_allocator() { return _allocator; }
		
		private:
			VkInstance _instance;
			VmaAllocator _allocator;
			VulkanDevice _vulkanDevice;

			vkb::Instance create_instance();
			void create_allocator();
			VkFormat get_texture_format(rhi::TextureFormat format);
			VmaMemoryUsage get_memory_usage(rhi::MemoryUsage memoryUsage);
			void get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo);
			VkBorderColor get_border_color(rhi::BorderColor borderColor);
			VkSamplerAddressMode get_address_mode(rhi::AddressMode addressMode);
			VkBufferUsageFlags get_buffer_usage(rhi::ResourceUsage usage);
			VkImageUsageFlags get_image_usage(rhi::ResourceUsage usage);
	};
}
