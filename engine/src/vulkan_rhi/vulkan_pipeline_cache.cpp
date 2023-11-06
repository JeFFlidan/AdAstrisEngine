#include "vulkan_pipeline_cache.h"
#include "vulkan_common.h"
#include "file_system/utils.h"

using namespace ad_astris::vulkan;

void VulkanPipelineCache::load_pipeline_cache(VulkanDevice* device, io::FileSystem* fileSystem)
{
	size_t size = 0;
	void* data = nullptr;
	
	if (io::Utils::exists(fileSystem->get_project_root_path(), "intermediate/pipeline_cache.bin"))
		data = fileSystem->map_to_read(fileSystem->get_project_root_path() + "/intermediate/pipeline_cache.bin", size);

	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	createInfo.initialDataSize = size;
	createInfo.pInitialData = data;

	VK_CHECK(vkCreatePipelineCache(device->get_device(), &createInfo, nullptr, &_pipelineCache));

	if (data)
		fileSystem->unmap_after_reading(data);
}

void VulkanPipelineCache::save_pipeline_cache(VulkanDevice* device, io::FileSystem* fileSystem)
{
	size_t cacheSize;
	vkGetPipelineCacheData(device->get_device(), _pipelineCache, &cacheSize, nullptr);
	std::vector<uint8_t> cacheData(cacheSize);
	vkGetPipelineCacheData(device->get_device(), _pipelineCache, &cacheSize, cacheData.data());

	io::URI cachePath = fileSystem->get_project_root_path() + "/intermediate/pipeline_cache.bin";
	io::Stream* stream = fileSystem->open(cachePath, "wb");
	stream->write(cacheData.data(), sizeof(uint8_t), cacheSize);
	fileSystem->close(stream);
}

void VulkanPipelineCache::destroy(VulkanDevice* device)
{
	if (_pipelineCache != VK_NULL_HANDLE)
		vkDestroyPipelineCache(device->get_device(), _pipelineCache, nullptr);
}
