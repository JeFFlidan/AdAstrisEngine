#include <stdint.h>
#include <vk_descriptors.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vulkan/vulkan_core.h>

void DescriptorAllocator::init(VkDevice newDevice)
{
	device = newDevice;
}

void DescriptorAllocator::cleanup()
{
	for (auto p : freePools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}

	for (auto p : usedPools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}
}

VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
{
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(poolSizes.sizes.size());
	for (auto sz : poolSizes.sizes)
	{
		sizes.push_back({ sz.first, uint32_t(sz.second * count) });
	}
	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = flags;
	pool_info.maxSets = count;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	VkDescriptorPool descriptorPool;
	vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

	return descriptorPool;
}

VkDescriptorPool DescriptorAllocator::grab_pool()
{
	if (freePools.size() > 0)
	{
		VkDescriptorPool pool = freePools.back();
		freePools.pop_back();
		return pool;
	}
	else
	{
		return createPool(device, descriptorSizes, 1000, 0);
	}
}

bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
	if (currentPool == VK_NULL_HANDLE)
	{
		currentPool = grab_pool();
		usedPools.push_back(currentPool);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;

	allocInfo.pSetLayouts = &layout;
	allocInfo.descriptorPool = currentPool;
	allocInfo.descriptorSetCount = 1;

	VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
	bool needReallocate = false;

	switch (allocResult)
	{
		case VK_SUCCESS:
			return true;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			needReallocate = true;
			break;
		default:
			return false;
	}

	if (needReallocate)
	{
		currentPool = grab_pool();
		usedPools.push_back(currentPool);

		allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
		if (allocResult == VK_SUCCESS)
			return true;
	}

	return false;
}

void DescriptorAllocator::reset_pools()
{
	for (auto p : usedPools)
	{
		vkResetDescriptorPool(device, p, 0);
		freePools.push_back(p);
	}

	usedPools.clear();

	currentPool = VK_NULL_HANDLE;
}

void DescriptorLayoutCache::init(VkDevice newDevice)
{
	device = newDevice;
}

void DescriptorLayoutCache::cleanup()
{
	for (auto pair : layoutCache)
	{
		vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
	}
}

VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info)
{
	DescriptorLayoutInfo layoutInfo;
	layoutInfo.bindings.reserve(info->bindingCount);
	bool isSorted = true;
	int lastBinding = -1;

	for (int i = 0; i != info->bindingCount; ++i)
	{
		layoutInfo.bindings.push_back(info->pBindings[i]);

		if (info->pBindings[i].binding > lastBinding)
		{
			lastBinding = info->pBindings[i].binding;
		}
		else
		{
			isSorted = false;
		}
	}

	if (!isSorted)
	{
		std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b){
			return a.binding < b.binding;
		});
	}

	auto it = layoutCache.find(layoutInfo);
	if (it != layoutCache.end())
	{
		return it->second;
	}
	else
	{
		VkDescriptorSetLayout layout;
		vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

		layoutCache[layoutInfo] = layout;

		return layout;
	}
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
	if (other.bindings.size() != bindings.size())
	{
		return false;
	}
	else
	{
		for (auto i = 0; i != bindings.size(); ++i)
		{
			if (other.bindings[i].binding != bindings[i].binding)
				return false;
			if (other.bindings[i].descriptorType != bindings[i].descriptorType)
				return false;
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
				return false;
			if (other.bindings[i].stageFlags != bindings[i].stageFlags)
				return false;
		}

		return true;
	}
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
	using std::size_t;
	using std::hash;

	size_t result = hash<size_t>()(bindings.size());

	for (const VkDescriptorSetLayoutBinding& b : bindings)
	{
		size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

		result ^= hash<size_t>()(binding_hash);
	}

	return result;
}

namespace vkutil
{
	DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
	{
		DescriptorBuilder builder;
		builder.allocator = allocator;
		builder.cache = layoutCache;

		return builder;
	}

	DescriptorBuilder& DescriptorBuilder::bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = descriptorCount;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;

		bindings.push_back(newBinding);

		VkWriteDescriptorSet newWrite{};
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;

		newWrite.descriptorCount = descriptorCount;
		newWrite.descriptorType = type;
		newWrite.dstBinding = binding;
		newWrite.pBufferInfo = bufferInfo;

		writes.push_back(newWrite);

		return *this;
	}

	DescriptorBuilder& DescriptorBuilder::bind_image(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = descriptorCount;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;

		bindings.push_back(newBinding);

		VkWriteDescriptorSet newWrite{};
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;

		newWrite.descriptorCount = descriptorCount;
		newWrite.descriptorType = type;
		newWrite.dstBinding = binding;
		newWrite.pImageInfo = imageInfo;

		writes.push_back(newWrite);

		return *this;
	}

	bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;

		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		layout = cache->create_descriptor_layout(&layoutInfo);

		bool success = allocator->allocate(&set, layout);
		if (!success) return false;

		for (VkWriteDescriptorSet& w : writes)
			w.dstSet = set;

		vkUpdateDescriptorSets(allocator->device, writes.size(), writes.data(), 0, nullptr);

		return true;
	}

	bool DescriptorBuilder::build_non_uniform(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
	{
		// This function uses to build non-uniform descriptor set to use it with VK_EXT_descriptor_indexing
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	    layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		const VkDescriptorBindingFlagsEXT flags =
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags{};
		bindingFlags.bindingCount = layoutInfo.bindingCount;
		bindingFlags.pBindingFlags = &flags;

		layoutInfo.pNext = &bindingFlags;

		layout = cache->create_descriptor_layout(&layoutInfo);
		bool success = allocator->allocate(&set, layout);
		if (!success) return false;

		for (VkWriteDescriptorSet& w : writes)
			w.dstSet = set;

		vkUpdateDescriptorSets(allocator->device, writes.size(), writes.data(), 0, nullptr);

		return true;
	}
}
