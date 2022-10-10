#include <fstream>
#include <stdint.h>
#include <vector>
#include <cassert>
#include <iostream>

#include <spirv_reflect.h>

#include "fmt/color.h"
#include "vk_shaders.h"
#include "logger.h"

namespace vkutil
{
	bool Shader::load_shader_module(const char* filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			return false;

		size_t fileSize = static_cast<size_t>(file.tellg());

		code.resize(fileSize / sizeof(uint32_t));

		file.seekg(0);

		file.read(reinterpret_cast<char*>(code.data()), fileSize);

		file.close();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;

		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = code.data();

		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			return false;

		return true;
	}

	void Shader::delete_shader_module()
	{
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}

	void Shader::spv_reflect_test()
	{
		SpvReflectShaderModule tempModule;
		uint32_t size = code.size() * sizeof(uint32_t);
		SpvReflectResult res = spvReflectCreateShaderModule(size, code.data(), &tempModule);
		assert(res == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t setCount = 0;
		res = spvReflectEnumerateDescriptorSets(&tempModule, &setCount, nullptr);
		assert(res == SPV_REFLECT_RESULT_SUCCESS);
	    
		LOG_INFO("Descriptor sets of shader count: {}", setCount);

		uint32_t bindCount = 0;
		res = spvReflectEnumerateDescriptorBindings(&tempModule, &bindCount, nullptr);
		assert(res == SPV_REFLECT_RESULT_SUCCESS);	    
		LOG_INFO("Descriptor binding count: {}", bindCount)

		std::vector<SpvReflectDescriptorBinding*> bindings(bindCount);
		res = spvReflectEnumerateDescriptorBindings(&tempModule, &bindCount, bindings.data());
		assert(res == SPV_REFLECT_RESULT_SUCCESS);
		LOG_INFO("Bindings vector size: {}", bindings.size())

		for (auto& bind : bindings)
		{
			LOG_INFO("Binding index {}", bind->input_attachment_index);
		}

		std::vector<SpvReflectDescriptorSet*> sets(setCount);
		res = spvReflectEnumerateDescriptorSets(&tempModule, &setCount, sets.data());
		assert(res == SPV_REFLECT_RESULT_SUCCESS);
		LOG_INFO("Sets vector size {}", sets.size());

		for (auto& set : sets)
	   	{
			LOG_INFO("Set bindings count {}", set->binding_count);
			LOG_INFO("Set {}", set->set);
		}
	}
} // namespace vkutil



