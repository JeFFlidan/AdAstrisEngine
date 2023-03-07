#include "vulkan_shader.h"
#include "profiler/logger.h"

#include <spirv_reflect.h>

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cassert>

using namespace ad_astris;

bool vulkan::VulkanShader::create_shader_module(rhi::ShaderInfo* shaderInfo)
{
	_code = shaderInfo->data;
	_size = shaderInfo->size;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	
	createInfo.codeSize = shaderInfo->size;
	createInfo.pCode = reinterpret_cast<uint32_t*>(_code);

	if (vkCreateShaderModule(_device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS)
	{
		LOG_ERROR("Failed to create shader module")
		return false;
	}

	return true;
}

void vulkan::VulkanShader::delete_shader_module()
{
	vkDestroyShaderModule(_device, _shaderModule, nullptr);
	_shaderModule = VK_NULL_HANDLE;
}

VkPipelineLayout vulkan::VulkanShaderStages::get_pipeline_layout(VkDevice device)
{
	std::unordered_map<int, std::vector<VkDescriptorSetLayoutBinding>> bindings;
	std::unordered_map<int, std::vector<VulkanShader::Binding>> bindingsCache;
	std::vector<VkPushConstantRange> ranges;

	for (auto& stage : stages)
	{
		auto code = stage.shader->get_code();
		auto size = stage.shader->get_size();

		SpvReflectShaderModule shaderModule;
		SpvReflectResult res = spvReflectCreateShaderModule(size, code, &shaderModule);
		assert(res == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t descriptorSetAmount;
		spvReflectEnumerateDescriptorSets(&shaderModule, &descriptorSetAmount, nullptr);
		std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetAmount);
		spvReflectEnumerateDescriptorSets(&shaderModule, &descriptorSetAmount, descriptorSets.data());

		std::sort(descriptorSets.begin(), descriptorSets.end(),
			[](SpvReflectDescriptorSet* first, SpvReflectDescriptorSet* second){
				if (first->set < second->set)
					return true;
				return false;
		});

		for (SpvReflectDescriptorSet* set : descriptorSets)
		{
			bool needsSorting = false;
			for (int i = 0; i != set->binding_count; ++i)
			{
				auto bind = set->bindings[i];
				
				VulkanShader::Binding binding;
				binding.binding = bind->binding;
				binding.pImmutableSamplers = nullptr;
				
				binding.stageFlags = stage.stage;
				binding.descriptorType = static_cast<VkDescriptorType>(bind->descriptor_type);

				if (bind->count == 0)
				{
					// It means that binding is non-uniform array
					binding.descriptorCount = 100;
				}
				else
				{
					binding.descriptorCount = bind->count;
				}

				auto& binds = bindingsCache[set->set];
				auto it = std::find(binds.begin(), binds.end(), binding);
				if (it == binds.end())
				{
					binds.push_back(binding);
				}
				else
				{
					it->stageFlags = it->stageFlags | binding.stageFlags;
				}
			}
		}

		uint32_t pushConstantsCount;
		spvReflectEnumeratePushConstantBlocks(&shaderModule, &pushConstantsCount, nullptr);	
		std::vector<SpvReflectBlockVariable*> blocks(pushConstantsCount);
		spvReflectEnumeratePushConstantBlocks(&shaderModule, &pushConstantsCount, blocks.data());

		if (pushConstantsCount > 0)
		{
			VkPushConstantRange range;
			range.offset = blocks[0]->offset;
			range.size = blocks[0]->size;
			range.stageFlags = stage.stage;
			ranges.push_back(range);
		}
	}

	for (auto& data : bindingsCache)
	{
		for (auto& bind : data.second)
		{
			VkDescriptorSetLayoutBinding binding;
			binding.binding = bind.binding;
			binding.descriptorCount = bind.descriptorCount;
			binding.pImmutableSamplers = bind.pImmutableSamplers;
			binding.stageFlags = bind.stageFlags;
			binding.descriptorType = bind.descriptorType;
			bindings[data.first].push_back(binding);
		}
	}

	for (auto& data : bindings)
	{
		auto& binds = data.second;
		std::sort(binds.begin(), binds.end(),
			[](VkDescriptorSetLayoutBinding& first, VkDescriptorSetLayoutBinding& second){
				if (first.binding < second.binding)
					return true;
				return false;
		});

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = binds.size();
		layoutInfo.pBindings = binds.data();
		if (binds.size() == 1 && binds[0].descriptorCount == 100)
		{
			// Setup flags for non-uniform texture array
			layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
			
			const VkDescriptorBindingFlags flags =
				VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
				VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
				VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
			bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlags.pNext = nullptr;
			bindingFlags.bindingCount = layoutInfo.bindingCount;
			bindingFlags.pBindingFlags = &flags;

			layoutInfo.pNext = &bindingFlags;
		}
		else
		{
			layoutInfo.pNext = nullptr;
		}

		VkDescriptorSetLayout layout;

		VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
		assert(res == VK_SUCCESS);

		layoutsCreateInfos.push_back(layoutInfo);
		setLayouts.push_back(layout);
	}

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = setLayouts.size();
	layoutInfo.pSetLayouts = setLayouts.data();
	layoutInfo.pushConstantRangeCount = ranges.size();
	layoutInfo.pPushConstantRanges = ranges.data();

	VkPipelineLayout layout;
	VkResult res = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout);
	assert(res == VK_SUCCESS);

	for (auto& set : setLayouts)
		vkDestroyDescriptorSetLayout(device, set, nullptr);

	return layout;
}