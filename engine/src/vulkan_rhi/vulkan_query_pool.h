#pragma once

#include "vulkan_object.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class VulkanQueryPool : public IVulkanObject
	{
		public:
			VulkanQueryPool() = default;
			VulkanQueryPool(VulkanDevice* device, rhi::QueryPoolInfo* queryPoolInfo);

			VkQueryPool get_handle() const { return _queryPool; }
			void destroy(VulkanDevice* device) override;

		private:
			VkQueryPool _queryPool{ VK_NULL_HANDLE };
	};
}