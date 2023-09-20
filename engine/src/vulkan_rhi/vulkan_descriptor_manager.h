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
			VulkanDescriptorManager(VulkanDevice* device, uint32_t buffersCount);
			~VulkanDescriptorManager();

			void cleanup();
		
			void allocate_bindless_descriptor(VulkanBuffer* buffer, uint32_t size, uint32_t offset);
			void allocate_bindless_descriptor(VulkanSampler* sampler);
			void allocate_bindless_descriptor(VulkanTextureView* textureView, TextureDescriptorHeapType heapType);
			void free_bindless_descriptor(VulkanBuffer* vulkanBuffer);
			VkDescriptorSetLayout get_bindless_descriptor_set_layout(VkDescriptorType descriptorType);
			VkDescriptorSet get_bindless_descriptor_set(VkDescriptorType descriptorType);

			void create_zero_descriptor_set(
				std::vector<VkDescriptorSetLayoutBinding>& inputBindings,
				std::vector<VkDescriptorSet>& outputDescriptorSets,
				VkDescriptorSetLayout& outputLayout);

			void allocate_uniform_buffer(VulkanBuffer* buffer, uint32_t size, uint32_t offset, uint32_t slot, uint32_t frameIndex);

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

			struct ZeroDescriptorSet
			{
				std::vector<VkDescriptorSet> descriptorSets;
				VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
				uint32_t bindingCount;
			};

			BindlessDescriptorPool _imageBindlessPool;
			BindlessDescriptorPool _storageImageBindlessPool;
			BindlessDescriptorPool _samplerBindlessPool;
			BindlessDescriptorPool _storageBufferBindlessPool;
			BindlessDescriptorPool _uniformTexelBufferBindlessPool;
			BindlessDescriptorPool _storageTexelBufferBindlessPool;
			VkDescriptorPool _zeroPool{ VK_NULL_HANDLE };
			std::unordered_map<uint64_t, ZeroDescriptorSet> _zeroDescriptorSetByItsHash;
			std::mutex _zeroSetsMutex;
		
			VulkanDevice* _device;
			uint32_t _buffersCount;

			void init_zero_pool();
			void cleanup_zero_pool();
	};
}