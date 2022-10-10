#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace vkutil
{
	class Shader
	{
		public:
			Shader(VkDevice dev) : device(dev) {}
			
			bool load_shader_module(const char* filePath);
			void delete_shader_module();
	 
			void spv_reflect_test();
		private:
			VkDevice device;
			VkShaderModule shaderModule;
			std::vector<uint32_t> code;
	};
}
