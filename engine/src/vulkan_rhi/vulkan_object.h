#pragma once

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class IVulkanObject
	{
		public:
			virtual void destroy(VulkanDevice* device) = 0;
	};
}