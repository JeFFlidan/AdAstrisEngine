#pragma once

#include "core/module.h"
#include "rhi/engine_rhi.h"

namespace ad_astris
{
	namespace vulkan
	{
		class VulkanRHI;
		class VulkanDevice;
		class VulkanQueue;
		class VulkanCommandManager;
	}
	
	class IVulkanRHIModule : public IModule
	{
		public:
			virtual rhi::IEngineRHI* create_vulkan_rhi() = 0;
	};
}