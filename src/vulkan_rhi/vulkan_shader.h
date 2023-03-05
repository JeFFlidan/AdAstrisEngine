#pragma once
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanShader
	{
		public:
			struct Binding
			{
				uint32_t binding;
				uint32_t descriptorCount;
				const VkSampler* pImmutableSamplers{nullptr};
				VkShaderStageFlags stageFlags;
				VkDescriptorType descriptorType;

				bool operator==(const Binding& other)
				{
					return binding == other.binding;
				}
			};

			VulkanShader(VkDevice device) : _device(device) {}
			~VulkanShader()
			{
				if (_shaderModule != VK_NULL_HANDLE)
					vkDestroyShaderModule(_device, _shaderModule, nullptr);
			}
		
			bool create_shader_module(rhi::ShaderInfo* shaderInfo);
				
			void delete_shader_module();

			VkShaderModule get_shader_module() const { return _shaderModule; }
			std::vector<uint32_t> get_code() const { return _code; }
		
		private:
			VkDevice _device;
			VkShaderModule _shaderModule{ VK_NULL_HANDLE };
			std::vector<uint32_t> _code;
	};
	
	struct VulkanShaderStages
	{
		// Storage information to create a VkPipeline
		std::vector<VkDescriptorSetLayout> setLayouts;
		std::vector<VkDescriptorSetLayoutCreateInfo> layoutsCreateInfos;

		struct ShaderStage
		{
			VulkanShader* shader;
			VkShaderStageFlagBits stage;
		};

		std::vector<ShaderStage> stages;

		void add_stage(VulkanShader* shader, VkShaderStageFlagBits shaderStage)
		{
			stages.push_back({shader, shaderStage});
		}

		VkPipelineLayout get_pipeline_layout(VkDevice device);
	};
}
