#include "vulkan_rhi.h"
#include "vulkan_ui_window_backend.h"
#include "engine/vulkan_rhi_module.h"
#include "core/global_objects.h"

namespace ad_astris::vulkan
{
	class VulkanRHIModule : public IVulkanRHIModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual rhi::RHI* create_vulkan_rhi() override;
			virtual rhi::UIWindowBackend* get_ui_window_backend() override;

		private:
			std::unique_ptr<VulkanRHI> _vulkanRHI{ nullptr };
			std::unique_ptr<VulkanUIWindowBackend> _backend{ nullptr };
	};

	void VulkanRHIModule::startup_module(ModuleManager* moduleManager)
	{
		_vulkanRHI = std::make_unique<VulkanRHI>();
		_backend = std::make_unique<VulkanUIWindowBackend>();
	}

	rhi::RHI* VulkanRHIModule::create_vulkan_rhi()
	{
		return _vulkanRHI.get();
	}

	rhi::UIWindowBackend* VulkanRHIModule::get_ui_window_backend()
	{
		return _backend.get();
	}

	DECL_MODULE_FUNCS(VulkanRHI, VulkanRHIModule)
}