﻿#pragma once

#include "vulkan_device.h"
#include "file_system/file_system.h"

namespace ad_astris::vulkan
{
	class VulkanPipelineCache
	{
		public:
			void load_pipeline_cache(VulkanDevice* device, io::FileSystem* fileSystem);
			void save_pipeline_cache(VulkanDevice* device, io::FileSystem* fileSystem);
			void destroy(VulkanDevice* device);
			VkPipelineCache get_handle() { return _pipelineCache; }

		private:
			VkPipelineCache _pipelineCache{ VK_NULL_HANDLE };
	};
}