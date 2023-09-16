#pragma once

#include "api.h"
#include "vulkan_device.h"
#include <mutex>

namespace ad_astris::vulkan
{
	class VulkanBuffer;
	class VulkanSampler;
	class VulkanTextureView;

	enum class TextureDescriptorHeapType
	{
		TEXTURES,
		STORAGE_TEXTURES,
	};
	
	class VulkanDescriptorManager
	{
		public:
			VulkanDescriptorManager(VulkanDevice* device);
			~VulkanDescriptorManager();

			void cleanup();
		
			void allocate_bindless_descriptor(VulkanBuffer* buffer, uint32_t size, uint32_t offset);
			void allocate_bindless_descriptor(VulkanSampler* sampler);
			void allocate_bindless_descriptor(VulkanTextureView* textureView, TextureDescriptorHeapType heapType);
			void free_bindless_descriptor(VulkanBuffer* vulkanBuffer);
			VkDescriptorSetLayout get_bindless_descriptor_set_layout(VkDescriptorType descriptorType);

		private:
			class BindlessDescriptorPool
			{
				public:
					void init(VulkanDevice* device, VkDescriptorType descriptorType, uint32_t descriptorCount);
					void cleanup(VulkanDevice* device);
					uint32_t allocate();
					void free(uint32_t descriptorIndex);
					
					VkDescriptorSetLayout get_layout()
					{
						return _layout;
					}

					VkDescriptorSet get_set()
					{
						return _set;
					}
				
				private:
					VkDescriptorPool _pool{ VK_NULL_HANDLE };
					VkDescriptorSet _set{ VK_NULL_HANDLE };
					VkDescriptorSetLayout _layout{ VK_NULL_HANDLE };
					std::vector<uint32_t> _freePlaces;
					std::mutex _mutex;
			};

			BindlessDescriptorPool _imageBindlessPool;
			BindlessDescriptorPool _storageImageBindlessPool;
			BindlessDescriptorPool _samplerBindlessPool;
			BindlessDescriptorPool _storageBufferBindlessPool;
			BindlessDescriptorPool _uniformTexelBufferBindlessPool;
			BindlessDescriptorPool _storageTexelBufferBindlessPool;
		
			VulkanDevice* _device;
	};
}