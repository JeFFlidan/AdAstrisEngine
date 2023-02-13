#pragma once

#include <stdint.h>
#include "vk_types.h"
#include <vector>
#include <array>
#include <unordered_map>

class DescriptorAllocator
{
	public:
		struct PoolSizes
		{
			std::vector<std::pair<VkDescriptorType, float>> sizes = 
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		void reset_pools();
		bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout, bool nonUniform = false, uint32_t descriptorsCount = 0);

		void init(VkDevice newDevice);

		void cleanup();

		VkDevice device;

		private:
			VkDescriptorPool grab_pool();

			VkDescriptorPool currentPool{ VK_NULL_HANDLE };
			PoolSizes descriptorSizes;
			std::vector<VkDescriptorPool> usedPools;
			std::vector<VkDescriptorPool> freePools;
};

class DescriptorLayoutCache
{
	public:
		void init(VkDevice device);
		void cleanup();

		VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};

	private:
		struct DescriptorLayoutHash
		{
			std::size_t operator()(const DescriptorLayoutInfo& k) const
			{
				return k.hash();
			}
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
		VkDevice device;
};

namespace ad_astris
{
	class DescriptorBuilder
	{
		public:
			static DescriptorBuilder begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator);

			DescriptorBuilder& bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t  descriptorCount = 1);
			DescriptorBuilder& bind_image(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1, uint32_t descriptorCountWrites = 1);

			bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
			bool build(VkDescriptorSet& set);
			bool build_non_uniform(VkDescriptorSet& set, VkDescriptorSetLayout& layout, uint32_t descriptorsCount);
			bool build_non_uniform(VkDescriptorSet& set, uint32_t descriptorCount);
			bool build_partially_bound(VkDescriptorSet& set, uint32_t descriptorsCount, uint32_t partiallyBoundedAmount);
		private:
			std::vector<VkWriteDescriptorSet> writes;
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			DescriptorLayoutCache* cache;
			DescriptorAllocator* allocator;
	};
}