#pragma once

#include "api.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanShader
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

			void cleanup()
			{
				if (_shaderModule != VK_NULL_HANDLE)
					vkDestroyShaderModule(_device, _shaderModule, nullptr);
			}
		
			bool create_shader_module(rhi::ShaderInfo* shaderInfo);
				
			void delete_shader_module();

			VkShaderModule get_shader_module() const { return _shaderModule; }
			uint8_t* get_code() const { return _code; }
			uint64_t get_size() const { return _size; }
		
		private:
			VkDevice _device;
			VkShaderModule _shaderModule{ VK_NULL_HANDLE };
			uint8_t* _code;
			uint64_t _size;
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
