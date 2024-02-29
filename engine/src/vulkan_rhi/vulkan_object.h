#pragma once

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class IVulkanObject
	{
		public:
			virtual ~IVulkanObject() = default;
			virtual void destroy(VulkanDevice* device) = 0;
	};
}