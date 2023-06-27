#include "vulkan_rhi.h"

#include "engine/vulkan_rhi_module.h"

namespace ad_astris::vulkan
{
	class VulkanRHIModule : public IVulkanRHIModule
	{
		public:
			VulkanRHI* create_vulkan_rhi() override;
	};

	VulkanRHI* VulkanRHIModule::create_vulkan_rhi()
	{
		return new VulkanRHI();
	}

	extern "C" VK_RHI_API IVulkanRHIModule* register_module()
	{
		return new VulkanRHIModule();
	}
}