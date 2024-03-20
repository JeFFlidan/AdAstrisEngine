#pragma once

#include "vulkan_device.h"
#include "file_system/file_system.h"

namespace ad_astris::vulkan
{
	class VulkanPipelineCache
	{
		public:
			void load_pipeline_cache(VulkanDevice* device);
			void save_pipeline_cache(VulkanDevice* device);
			void destroy(VulkanDevice* device);
			VkPipelineCache get_handle() { return _pipelineCache; }

		private:
			VkPipelineCache _pipelineCache{ VK_NULL_HANDLE };

			bool is_loaded_cache_valid(VulkanDevice* device, uint8_t* cacheData);
	};
}
