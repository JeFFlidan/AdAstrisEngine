#define NOMINMAX
#include "vulkan_descriptor_manager.h"
#include "vulkan_common.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "profiler/logger.h"
#include "core/utils.h"
#include <algorithm>

using namespace ad_astris::vulkan;

constexpr uint32_t MAX_BINDLESS_DESCRIPTORS = 100000u;
constexpr uint32_t MAX_UNIFORM_BUFFERS = 16u;
constexpr uint32_t MAX_ZERO_SETS = 64;

VulkanDescriptorManager::VulkanDescriptorManager(VulkanDevice* device, uint32_t buffersCount)
	: _device(device), _buffersCount(buffersCount)
{
	VkPhysicalDeviceVulkan12Properties& properties = _device->get_physical_device_vulkan_1_2_properties();
	_imageBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min(MAX_BINDLESS_DESCRIPTORS, properties.maxDescriptorSetUpdateAfterBindSampledImages / 4));
	_storageImageBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, std::min(MAX_BINDLESS_DESCRIPTORS, properties.maxDescriptorSetUpdateAfterBindStorageImages / 4));
	_storageBufferBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(MAX_BINDLESS_DESCRIPTORS, properties.maxDescriptorSetUpdateAfterBindStorageBuffers / 4));
	_uniformTexelBufferBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, std::min(MAX_BINDLESS_DESCRIPTORS, properties.maxDescriptorSetUpdateAfterBindSampledImages / 4));
	_storageTexelBufferBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, std::min(MAX_BINDLESS_DESCRIPTORS, properties.maxDescriptorSetUpdateAfterBindStorageBuffers / 4));
	_samplerBindlessPool.init(_device, VK_DESCRIPTOR_TYPE_SAMPLER, 256);

	init_zero_pool();
}

VulkanDescriptorManager::~VulkanDescriptorManager()
{
	
}

void VulkanDescriptorManager::cleanup()
{
	_imageBindlessPool.cleanup(_device);
	_storageBufferBindlessPool.cleanup(_device);
	_storageImageBindlessPool.cleanup(_device);
	_uniformTexelBufferBindlessPool.cleanup(_device);
	_storageTexelBufferBindlessPool.cleanup(_device);
	_samplerBindlessPool.cleanup(_device);
	cleanup_zero_pool();
}

void VulkanDescriptorManager::allocate_bindless_descriptor(VulkanBuffer* buffer, uint32_t size, uint32_t offset)
{
	uint32_t bufferDescriptorIndex = _storageBufferBindlessPool.allocate();
	buffer->set_descriptor_index(bufferDescriptorIndex);
	
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = *buffer->get_handle();
	bufferInfo.offset = offset;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstSet = _storageBufferBindlessPool.get_set();
	writeDescriptorSet.dstArrayElement = bufferDescriptorIndex;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(_device->get_device(), 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorManager::allocate_bindless_descriptor(VulkanSampler* sampler)
{
	uint32_t samplerDescriptorIndex = _samplerBindlessPool.allocate();
	sampler->set_descriptor_index(samplerDescriptorIndex);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = sampler->get_handle();

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstSet = _samplerBindlessPool.get_set();
	writeDescriptorSet.dstArrayElement = samplerDescriptorIndex;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(_device->get_device(), 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorManager::allocate_bindless_descriptor(VulkanTextureView* textureView, TextureDescriptorHeapType heapType)
{
	uint32_t textureDescriptorIndex = _samplerBindlessPool.allocate();
	textureView->set_descriptor_index(textureDescriptorIndex);

	switch (heapType)
	{
		case TextureDescriptorHeapType::TEXTURES:
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = textureView->get_handle();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.dstSet = _imageBindlessPool.get_set();
			writeDescriptorSet.dstArrayElement = textureDescriptorIndex;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(_device->get_device(), 1, &writeDescriptorSet, 0, nullptr);
		}
		case TextureDescriptorHeapType::STORAGE_TEXTURES:
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.imageView = textureView->get_handle();

			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.dstSet = _storageImageBindlessPool.get_set();
			writeDescriptorSet.dstArrayElement = textureDescriptorIndex;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(_device->get_device(), 1, &writeDescriptorSet, 0, nullptr);
		}
	}
}

void VulkanDescriptorManager::free_bindless_descriptor(VulkanBuffer* vulkanBuffer)
{
	_storageBufferBindlessPool.free(vulkanBuffer->get_descriptor_index());
	vulkanBuffer->set_descriptor_index(~0u);
}

VkDescriptorSetLayout VulkanDescriptorManager::get_bindless_descriptor_set_layout(VkDescriptorType descriptorType)
{
	switch (descriptorType)
	{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			return _samplerBindlessPool.get_layout();
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return _storageBufferBindlessPool.get_layout();
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return _imageBindlessPool.get_layout();
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return _storageImageBindlessPool.get_layout();
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			return _uniformTexelBufferBindlessPool.get_layout();
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			return _storageTexelBufferBindlessPool.get_layout();
	}

	LOG_FATAL("VulkanDescriptorManager::get_bindless_descriptor_layout(): No bindless descriptor layout")
}

VkDescriptorSet VulkanDescriptorManager::get_bindless_descriptor_set(VkDescriptorType descriptorType)
{
	switch (descriptorType)
	{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			return _samplerBindlessPool.get_set();
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return _storageBufferBindlessPool.get_set();
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return _imageBindlessPool.get_set();
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return _storageImageBindlessPool.get_set();
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			return _uniformTexelBufferBindlessPool.get_set();
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			return _storageTexelBufferBindlessPool.get_set();
	}

	LOG_FATAL("VulkanDescriptorManager::get_bindless_descriptor_set(): No bindless descriptor set")
}

void VulkanDescriptorManager::create_zero_descriptor_set(
	std::vector<VkDescriptorSetLayoutBinding>& inputBindings,
	std::vector<VkDescriptorSet>& outputDescriptorSets,
	VkDescriptorSetLayout& outputLayout)
{
	uint64_t hash = 0;
	for (auto& binding : inputBindings)
	{
		CoreUtils::hash_combine(hash, binding.binding);
		CoreUtils::hash_combine(hash, binding.descriptorCount);
		CoreUtils::hash_combine(hash, binding.descriptorType);
		CoreUtils::hash_combine(hash, binding.stageFlags);
	}

	std::scoped_lock<std::mutex> locker(_zeroSetsMutex);
	auto it = _zeroDescriptorSetByItsHash.find(hash);
	if (it != _zeroDescriptorSetByItsHash.end())
	{
		for (auto& descriptorSet : it->second.descriptorSets)
			outputDescriptorSets.push_back(descriptorSet);
		outputLayout = it->second.layout;
		return;
	}

	_zeroDescriptorSetByItsHash[hash] = ZeroDescriptorSet();
	auto& zeroSet = _zeroDescriptorSetByItsHash[hash];
	zeroSet.bindingCount = inputBindings.size();
	
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = inputBindings.size();
	layoutCreateInfo.pBindings = inputBindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(_device->get_device(), &layoutCreateInfo, nullptr, &zeroSet.layout));
	outputLayout = zeroSet.layout;

	zeroSet.descriptorSets.resize(_buffersCount);
	for (uint32_t i = 0; i != _buffersCount; ++i)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _zeroPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &zeroSet.layout;

		VK_CHECK(vkAllocateDescriptorSets(_device->get_device(), &allocInfo, &zeroSet.descriptorSets[i]));
		outputDescriptorSets.push_back(zeroSet.descriptorSets[i]);
	}
}

void VulkanDescriptorManager::allocate_uniform_buffer(VulkanBuffer* buffer, uint32_t size, uint32_t offset, uint32_t slot, uint32_t frameIndex)
{
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = *buffer->get_handle();
	bufferInfo.offset = offset;
	bufferInfo.range = size;

	std::vector<VkWriteDescriptorSet> writes;
	
	for (auto& zeroSetInfo : _zeroDescriptorSetByItsHash)
	{
		if (slot > zeroSetInfo.second.bindingCount)
			continue;

		auto& writeDescriptorSet = writes.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = slot;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstSet = zeroSetInfo.second.descriptorSets[frameIndex];
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
	}

	vkUpdateDescriptorSets(_device->get_device(), writes.size(), writes.data(), 0, nullptr);
}

void VulkanDescriptorManager::init_zero_pool()
{
	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = MAX_UNIFORM_BUFFERS * MAX_ZERO_SETS;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = MAX_ZERO_SETS;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	VK_CHECK(vkCreateDescriptorPool(_device->get_device(), &poolCreateInfo, nullptr, &_zeroPool));
}

void VulkanDescriptorManager::cleanup_zero_pool()
{
	if (_zeroPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_device->get_device(), _zeroPool, nullptr);
	}
	for (auto& zeroSet : _zeroDescriptorSetByItsHash)
	{
		if (zeroSet.second.layout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(_device->get_device(), zeroSet.second.layout, nullptr);
	}
}

void VulkanDescriptorManager::BindlessDescriptorPool::init(VulkanDevice* device, VkDescriptorType descriptorType, uint32_t descriptorCount)
{
	VkDescriptorPoolSize poolSize;
	poolSize.type = descriptorType;
	poolSize.descriptorCount = descriptorCount;
	
	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolCreateInfo.maxSets = 1;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	VK_CHECK(vkCreateDescriptorPool(device->get_device(), &poolCreateInfo, nullptr, &_pool));

	VkDescriptorSetLayoutBinding binding;
	binding.binding = 0;
	binding.descriptorType = descriptorType;
	binding.descriptorCount = descriptorCount;
	binding.stageFlags = VK_SHADER_STAGE_ALL;
	binding.pImmutableSamplers = nullptr;

	VkDescriptorBindingFlags bindingFlags =
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
		VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
	VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlags{};
	layoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	layoutBindingFlags.pBindingFlags = &bindingFlags;
	layoutBindingFlags.bindingCount = 1;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &binding;
	layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	layoutCreateInfo.pNext = &layoutBindingFlags;
	VK_CHECK(vkCreateDescriptorSetLayout(device->get_device(), &layoutCreateInfo, nullptr, &_layout));

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_layout;
	allocInfo.descriptorPool = _pool;
	VK_CHECK(vkAllocateDescriptorSets(device->get_device(), &allocInfo, &_set));

	for (int32_t i = 0; i != (int32_t)descriptorCount; ++i)
	{
		_freePlaces.push_back((int32_t)descriptorCount - i - 1);
	}
}

void VulkanDescriptorManager::BindlessDescriptorPool::cleanup(VulkanDevice* device)
{
	if (_pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device->get_device(), _pool, nullptr);
		_pool = VK_NULL_HANDLE;
	}
	if (_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device->get_device(), _layout, nullptr);
		_layout = VK_NULL_HANDLE;
	}
}

uint32_t VulkanDescriptorManager::BindlessDescriptorPool::allocate()
{
	std::scoped_lock<std::mutex> locker(_mutex);
	if (_freePlaces.empty())
		LOG_FATAL("VulkanDescriptorManager::BindlessDescriptorPool::allocate(): There are no free places in the descriptor pool")
	uint32_t index = _freePlaces.back();
	_freePlaces.pop_back();
	return index;
}

void VulkanDescriptorManager::BindlessDescriptorPool::free(uint32_t descriptorIndex)
{
	std::scoped_lock<std::mutex> locker(_mutex);
	if (descriptorIndex > 0)
		_freePlaces.push_back(descriptorIndex);
}
