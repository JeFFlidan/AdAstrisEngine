#pragma once

#include "core/module.h"
#include "rhi/engine_rhi.h"
#include "rhi/ui_window_backend.h"

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
			virtual rhi::RHI* create_vulkan_rhi() = 0;
			virtual rhi::UIWindowBackend* get_ui_window_backend() = 0;
	};
}