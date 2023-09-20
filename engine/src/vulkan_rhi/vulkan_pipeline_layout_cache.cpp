#define NOMINMAX
#include "vulkan_pipeline_layout_cache.h"
#include "vulkan_common.h"

#include "core/utils.h"

#include <algorithm>

using namespace ad_astris;
using namespace vulkan;

VulkanPipelineLayout::VulkanPipelineLayout(
	VulkanDevice* device,
	VulkanPipelineLayoutIntermediateContext& intermediateContext,
	VulkanDescriptorManager* descriptorManager)
{
	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	auto& zeroLayout = descriptorLayouts.emplace_back();
	descriptorManager->create_zero_descriptor_set(intermediateContext.zeroSetBindings, _zeroDescriptorSets, zeroLayout);
	_firstBindlessSet = descriptorLayouts.size();

	for (auto& bindlessBinding : intermediateContext.bindlessBindings)
	{
		descriptorLayouts.push_back(descriptorManager->get_bindless_descriptor_set_layout(bindlessBinding.descriptorType));
		_bindlessDescriptorSets.push_back(descriptorManager->get_bindless_descriptor_set(bindlessBinding.descriptorType));
	}

	_pushConstantRange = intermediateContext.pushConstant;

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pSetLayouts = descriptorLayouts.data();
	createInfo.setLayoutCount = descriptorLayouts.size();
	if (_pushConstantRange.size > 0)
	{
		createInfo.pPushConstantRanges = &_pushConstantRange;
		createInfo.pushConstantRangeCount = 1;
	}
	else
	{
		createInfo.pPushConstantRanges = nullptr;
		createInfo.pushConstantRangeCount = 0;
	}
	VK_CHECK(vkCreatePipelineLayout(device->get_device(), &createInfo, nullptr, &_layout));
}

VulkanPipelineLayout::VulkanPipelineLayout(
	VulkanDevice* device,
	VulkanShaderReflectContext& reflectContext,
	VulkanDescriptorManager* descriptorManager)
{
	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	auto& zeroLayout = descriptorLayouts.emplace_back();
	descriptorManager->create_zero_descriptor_set(reflectContext.zeroSetBindings, _zeroDescriptorSets, zeroLayout);
	_firstBindlessSet = descriptorLayouts.size();

	for (auto& bindlessBinding : reflectContext.bindlessBindings)
	{
		if (!bindlessBinding.isUsed)
			continue;
		descriptorLayouts.push_back(descriptorManager->get_bindless_descriptor_set_layout(bindlessBinding.binding.descriptorType));
		_bindlessDescriptorSets.push_back(descriptorManager->get_bindless_descriptor_set(bindlessBinding.binding.descriptorType));
	}

	_pushConstantRange = reflectContext.pushConstantRange;

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pSetLayouts = descriptorLayouts.data();
	createInfo.setLayoutCount = descriptorLayouts.size();
	if (_pushConstantRange.size > 0)
	{
		createInfo.pPushConstantRanges = &_pushConstantRange;
		createInfo.pushConstantRangeCount = 1;
	}
	else
	{
		createInfo.pPushConstantRanges = nullptr;
		createInfo.pushConstantRangeCount = 1;
	}
	VK_CHECK(vkCreatePipelineLayout(device->get_device(), &createInfo, nullptr, &_layout));
}

void VulkanPipelineLayout::bind_descriptor_sets(VkCommandBuffer cmd, uint32_t frameIndex, VkPipelineBindPoint bindPoint)
{
	if (!_zeroDescriptorSets.empty())
		vkCmdBindDescriptorSets(cmd, bindPoint, _layout, 0, 1, &_zeroDescriptorSets[frameIndex], 0, nullptr);
	if (!_bindlessDescriptorSets.empty())
		vkCmdBindDescriptorSets(cmd, bindPoint, _layout, _firstBindlessSet, _bindlessDescriptorSets.size(), _bindlessDescriptorSets.data(), 0, nullptr);
}

void VulkanPipelineLayout::push_constant(VkCommandBuffer cmd, void* data)
{
	vkCmdPushConstants(cmd, _layout, _pushConstantRange.stageFlags, _pushConstantRange.offset, _pushConstantRange.size, data);
}

void VulkanPipelineLayout::destroy(VulkanDevice* device)
{
	vkDestroyPipelineLayout(device->get_device(), _layout, nullptr);
}

VulkanPipelineLayoutCache::VulkanPipelineLayoutCache(VulkanDevice* device, VulkanDescriptorManager* descriptorManager)
	: _descriptorManager(descriptorManager), _device(device)
{
	
}

void VulkanPipelineLayoutCache::cleanup()
{
	for (auto& pair : _layoutByItsHash)
	{
		pair.second->destroy(_device);
	}
}

VulkanPipelineLayout* VulkanPipelineLayoutCache::get_layout(std::vector<rhi::Shader>& rhiShaders)
{
	VulkanPipelineLayoutIntermediateContext intermediateContext;
	merge_shader_reflects(rhiShaders, intermediateContext);

	{
		std::scoped_lock<std::mutex> locker(_mutex);
		auto it = _layoutByItsHash.find(intermediateContext.layoutHash);
		if (it != _layoutByItsHash.end())
			return it->second.get();
	}

	auto layout = std::make_unique<VulkanPipelineLayout>(_device, intermediateContext, _descriptorManager);
		{
		std::scoped_lock<std::mutex> locker(_mutex);
		_layoutByItsHash[intermediateContext.layoutHash] = std::move(layout);
		return _layoutByItsHash[intermediateContext.layoutHash].get();
	}
}

VulkanPipelineLayout* VulkanPipelineLayoutCache::get_layout(rhi::Shader& rhiShader)
{
	auto reflectContext = get_vk_obj(&rhiShader)->get_reflect_context();
	uint64_t layoutHash = 0;
	for (auto& bindlessBinding : reflectContext->bindlessBindings)
	{
		CoreUtils::hash_combine(layoutHash, bindlessBinding.binding.binding);
		CoreUtils::hash_combine(layoutHash, bindlessBinding.binding.descriptorCount);
		CoreUtils::hash_combine(layoutHash, bindlessBinding.binding.descriptorType);
		CoreUtils::hash_combine(layoutHash, bindlessBinding.binding.stageFlags);
	}

	for (auto& binding : reflectContext->zeroSetBindings)
	{
		CoreUtils::hash_combine(layoutHash, binding.binding);
		CoreUtils::hash_combine(layoutHash, binding.descriptorCount);
		CoreUtils::hash_combine(layoutHash, binding.descriptorType);
		CoreUtils::hash_combine(layoutHash, binding.stageFlags);
	}

	CoreUtils::hash_combine(layoutHash, reflectContext->pushConstantRange.offset);
	CoreUtils::hash_combine(layoutHash, reflectContext->pushConstantRange.size);
	CoreUtils::hash_combine(layoutHash, reflectContext->pushConstantRange.stageFlags);

	{
		std::scoped_lock<std::mutex> locker(_mutex);
		auto it = _layoutByItsHash.find(layoutHash);
		if (it != _layoutByItsHash.end())
			return it->second.get();
	}
	
	auto& reflectContextRef = const_cast<VulkanShaderReflectContext&>(*reflectContext);
	auto layout = std::make_unique<VulkanPipelineLayout>(_device, reflectContextRef, _descriptorManager);
	
	{
		std::scoped_lock<std::mutex> locker(_mutex);
		_layoutByItsHash[layoutHash] = std::move(layout);
		return _layoutByItsHash[layoutHash].get();
	}
}

void VulkanPipelineLayoutCache::merge_shader_reflects(
	std::vector<rhi::Shader>& shaders,
	VulkanPipelineLayoutIntermediateContext& intermediateContext)
{
	for (auto& shader : shaders)
	{
		auto reflectContext = get_vk_obj(&shader)->get_reflect_context();
		intermediateContext.bindlessBindings.resize(std::max(
			intermediateContext.bindlessBindings.size(),
			reflectContext->bindlessBindings.size()));

		for (uint32_t i = 0; i != reflectContext->bindlessBindings.size(); ++i)
		{
			auto& reflectContextBindlessBinding = reflectContext->bindlessBindings[i];
			auto& interContextBindlessBinding = intermediateContext.bindlessBindings[i];
			
			if (!reflectContextBindlessBinding.isUsed)
				continue;

			if (reflectContextBindlessBinding.binding.descriptorType != interContextBindlessBinding.descriptorType)
			{
				intermediateContext.bindlessBindings[i] = reflectContextBindlessBinding.binding;
			}
			else
			{
				interContextBindlessBinding.stageFlags |= reflectContextBindlessBinding.binding.stageFlags;
			}
		}

		intermediateContext.zeroSetBindings.resize(std::max(
			intermediateContext.zeroSetBindings.size(),
			reflectContext->zeroSetBindings.size()));

		for (uint32_t i = 0; i != reflectContext->zeroSetBindings.size(); ++i)
		{
			auto& reflectContextBinding = reflectContext->zeroSetBindings[i];
			auto& interContextBinding = intermediateContext.zeroSetBindings[i];

			if (reflectContextBinding.descriptorType != interContextBinding.descriptorType)
			{
				intermediateContext.zeroSetBindings[i] = reflectContextBinding;
			}
			else
			{
				intermediateContext.zeroSetBindings[i].stageFlags |= reflectContextBinding.stageFlags;
			}
		}

		if (reflectContext->isPushConstantUsed)
		{
			intermediateContext.pushConstant.offset = std::min(
				intermediateContext.pushConstant.offset,
				reflectContext->pushConstantRange.offset);
			intermediateContext.pushConstant.size = std::max(
				intermediateContext.pushConstant.size,
				reflectContext->pushConstantRange.size);
			intermediateContext.pushConstant.stageFlags |= reflectContext->pushConstantRange.stageFlags;
		}
		else
		{
			intermediateContext.pushConstant.size = 0;
		}
	}

	intermediateContext.layoutHash = 0;

	for (auto& bindlessBinding : intermediateContext.bindlessBindings)
	{
		CoreUtils::hash_combine(intermediateContext.layoutHash, bindlessBinding.binding);
		CoreUtils::hash_combine(intermediateContext.layoutHash, bindlessBinding.descriptorCount);
		CoreUtils::hash_combine(intermediateContext.layoutHash, bindlessBinding.descriptorType);
		CoreUtils::hash_combine(intermediateContext.layoutHash, bindlessBinding.stageFlags);
	}

	for (auto& binding : intermediateContext.zeroSetBindings)
	{
		CoreUtils::hash_combine(intermediateContext.layoutHash, binding.binding);
		CoreUtils::hash_combine(intermediateContext.layoutHash, binding.descriptorCount);
		CoreUtils::hash_combine(intermediateContext.layoutHash, binding.descriptorType);
		CoreUtils::hash_combine(intermediateContext.layoutHash, binding.stageFlags);
	}

	if (intermediateContext.pushConstant.size)
	{
		CoreUtils::hash_combine(intermediateContext.layoutHash, intermediateContext.pushConstant.offset);
		CoreUtils::hash_combine(intermediateContext.layoutHash, intermediateContext.pushConstant.size);
		CoreUtils::hash_combine(intermediateContext.layoutHash, intermediateContext.pushConstant.stageFlags);
	}
}
