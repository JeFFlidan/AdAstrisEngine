#pragma once

#include "core/module.h"
#include "rhi/engine_rhi.h"
#include "rhi/imgui_backend.h"

namespace ad_astris::vulkan
{
	class IVulkanRHIModule : public IModule
	{
		public:
			virtual rhi::RHI* create_vulkan_rhi() = 0;
			virtual rhi::IImGuiBackend* get_imgui_backend() = 0;
	};
}