#pragma once

#include "vulkan_device.h"

namespace ad_astris::vulkan
{
	class VulkanDescriptorManager
	{
		public:
			VulkanDescriptorManager(VulkanDevice* device);
			~VulkanDescriptorManager();

		private:
			VulkanDevice* _device;
	};
}