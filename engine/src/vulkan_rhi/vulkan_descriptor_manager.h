#pragma once

#include "api.h"
#include "vulkan_device.h"

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanDescriptorManager
	{
		public:
			VulkanDescriptorManager(VulkanDevice* device);
			~VulkanDescriptorManager();

		private:
			VulkanDevice* _device;
	};
}