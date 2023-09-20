#pragma once

#include "vulkan_descriptor_manager.h"
#include "vulkan_shader.h"
#include <memory>

namespace ad_astris::vulkan
{
	struct VulkanPipelineLayoutIntermediateContext
	{
		std::vector<VkDescriptorSetLayoutBinding> zeroSetBindings;
		std::vector<VkDescriptorSetLayoutBinding> bindlessBindings;
		VkPushConstantRange pushConstant;
		uint64_t layoutHash;
	};
	
	class VulkanPipelineLayout
	{
		public:
			// For graphics pipeline layout
			VulkanPipelineLayout(
				VulkanDevice* device,
				VulkanPipelineLayoutIntermediateContext& intermediateContext,
				VulkanDescriptorManager* descriptorManager);
			// For compute pipeline layout
			VulkanPipelineLayout(
				VulkanDevice* device,
				VulkanShaderReflectContext& reflectContext,
				VulkanDescriptorManager* descriptorManager);

			void bind_descriptor_sets(VkCommandBuffer cmd, uint32_t frameIndex, VkPipelineBindPoint bindPoint);
			void push_constant(VkCommandBuffer cmd, void* data);
			VkPipelineLayout get_handle() { return _layout; }
			void destroy(VulkanDevice* device);

		private:
			VkPipelineLayout _layout{ VK_NULL_HANDLE };
			std::vector<VkDescriptorSet> _zeroDescriptorSets;
			std::vector<VkDescriptorSet> _bindlessDescriptorSets;
			VkPushConstantRange _pushConstantRange;
			uint32_t _firstBindlessSet;
	};

	class VulkanPipelineLayoutCache
	{
		public:
			VulkanPipelineLayoutCache(VulkanDevice* device, VulkanDescriptorManager* descriptorManager);
			void cleanup();

			VulkanPipelineLayout* get_layout(std::vector<rhi::Shader>& rhiShaders);
			VulkanPipelineLayout* get_layout(rhi::Shader& rhiShader);

		private:
			VulkanDevice* _device{ nullptr };
			VulkanDescriptorManager* _descriptorManager{ nullptr };
			std::unordered_map<uint64_t, std::unique_ptr<VulkanPipelineLayout>> _layoutByItsHash;
			std::mutex _mutex;

			void merge_shader_reflects(
				std::vector<rhi::Shader>& rhiShaders,
				VulkanPipelineLayoutIntermediateContext& intermediateContext);
	};
}
