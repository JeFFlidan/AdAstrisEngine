#include "vulkan_rhi.h"

#include "engine/vulkan_rhi_module.h"

namespace ad_astris::vulkan
{
	class VulkanRHIModule : public IVulkanRHIModule
	{
		public:
			rhi::IEngineRHI* create_vulkan_rhi() override;

		private:
			std::unique_ptr<VulkanRHI> _vulkanRHI{ nullptr };
	};

	rhi::IEngineRHI* VulkanRHIModule::create_vulkan_rhi()
	{
		if (!_vulkanRHI)
		{
			_vulkanRHI = std::make_unique<VulkanRHI>();
		}
		
		return _vulkanRHI.get();
	}

	extern "C" VK_RHI_API IVulkanRHIModule* register_module()
	{
		return new VulkanRHIModule();
	}
}