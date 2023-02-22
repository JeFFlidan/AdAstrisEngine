#pragma once

#include <stdint.h>
#include <vector>

#include <vulkan/vulkan.h>

namespace ad_astris
{
	class Shader
	{
		public:
			Shader(VkDevice dev) : device(dev) {}
			
			bool load_shader_module(const char* filePath);
			
			void delete_shader_module();

			VkShaderModule get_shader_module() const { return shaderModule; }
			std::vector<uint32_t> get_code() const { return code; }
		private:
			VkDevice device;
			VkShaderModule shaderModule;
			std::vector<uint32_t> code;
	};
	
	struct ShaderEffect
	{
		// Storage information to create a VkPipeline
		//VkPipelineLayout buildLayout;
		std::vector<VkDescriptorSetLayout> setLayouts;
		std::vector<VkDescriptorSetLayoutCreateInfo> layoutsCreateInfos;

		struct ShaderStage
		{
			Shader* shader;
			VkShaderStageFlagBits stage;
		};

		std::vector<ShaderStage> stages;

		void add_stage(Shader* shader, VkShaderStageFlagBits shaderStage)
		{
			stages.push_back({shader, shaderStage});
		}

		VkPipelineLayout get_pipeline_layout(VkDevice device);
		void destroy_shader_modules();
	};

}
