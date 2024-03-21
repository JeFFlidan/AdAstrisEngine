#include "vulkan_pipeline_cache.h"
#include "vulkan_common.h"
#include "file_system/utils.h"
#include "core/global_objects.h"

using namespace ad_astris::vulkan;

void VulkanPipelineCache::load_pipeline_cache(VulkanDevice* device)
{
	size_t size = 0;
	void* data = nullptr;
	
	if (io::Utils::exists(FILE_SYSTEM()->get_project_root_path(), "intermediate/pipeline_cache.bin"))
	{
		data = FILE_SYSTEM()->map_to_read(FILE_SYSTEM()->get_project_root_path() + "/intermediate/pipeline_cache.bin", size);
		if (!is_loaded_cache_valid(device, static_cast<uint8_t*>(data)))
		{
			LOG_WARNING("VulkanPipelineCache::load_pipeline_cache(): Pipeline cache is invalid")
			FILE_SYSTEM()->unmap_after_reading(data);
			data = nullptr;
			size = 0;
		}
	}

	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	createInfo.initialDataSize = size;
	createInfo.pInitialData = data;

	VK_CHECK(vkCreatePipelineCache(device->get_device(), &createInfo, nullptr, &_pipelineCache));

	if (data)
		FILE_SYSTEM()->unmap_after_reading(data);
}

void VulkanPipelineCache::save_pipeline_cache(VulkanDevice* device)
{
	size_t cacheSize;
	vkGetPipelineCacheData(device->get_device(), _pipelineCache, &cacheSize, nullptr);
	std::vector<uint8_t> cacheData(cacheSize);
	vkGetPipelineCacheData(device->get_device(), _pipelineCache, &cacheSize, cacheData.data());

	io::URI cachePath = FILE_SYSTEM()->get_project_root_path() + "/intermediate/pipeline_cache.bin";
	io::Stream* stream = FILE_SYSTEM()->open(cachePath, "wb");
	stream->write(cacheData.data(), sizeof(uint8_t), cacheSize);
	FILE_SYSTEM()->close(stream);
}

void VulkanPipelineCache::destroy(VulkanDevice* device)
{
	if (_pipelineCache != VK_NULL_HANDLE)
		vkDestroyPipelineCache(device->get_device(), _pipelineCache, nullptr);
}

bool VulkanPipelineCache::is_loaded_cache_valid(VulkanDevice* device, uint8_t* cacheData)
{
	uint32_t headerLength{ 0 };
	uint32_t cacheHeaderVersion{ 0 };
	uint32_t vendorID{ 0 };
	uint32_t deviceID{ 0 };
	uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};
	
	memcpy(&headerLength, cacheData, 4);
	memcpy(&cacheHeaderVersion, cacheData + 4, 4);
	memcpy(&vendorID, cacheData + 8, 4);
	memcpy(&deviceID, cacheData + 12, 4);
	memcpy(pipelineCacheUUID, cacheData + 16, VK_UUID_SIZE);

	const VkPhysicalDeviceProperties& properties = device->get_physical_device_properties().properties;
	
	if (headerLength <= 0)
		return false;
	if (cacheHeaderVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE)
		return false;
	if (vendorID != properties.vendorID)
		return false;
	if (deviceID != properties.deviceID)
		return false;
	if ((memcmp(pipelineCacheUUID, properties.pipelineCacheUUID, sizeof(pipelineCacheUUID))) != 0)
		return false;
	return true;
}
