#pragma once

#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "application_core/window.h"

namespace ad_astris::vulkan
{
	class VulkanSurface
	{
		public:
			VulkanSurface(VulkanInstance* instance, VulkanDevice* device, acore::IWindow* window);
			void cleanup(VulkanInstance* instance);

			VkSurfaceKHR get_handle() const { return _surface; }
			VkSurfaceCapabilitiesKHR get_capabilities(VulkanDevice* device) const;
			const std::vector<VkSurfaceFormat2KHR>& get_available_formats() const { return _availableFormats; }
			const std::vector<VkPresentModeKHR>& get_available_present_modes() const { return _availablePresentModes; }

		private:
			VkSurfaceKHR _surface{ VK_NULL_HANDLE };
			std::vector<VkSurfaceFormat2KHR> _availableFormats;
			std::vector<VkPresentModeKHR> _availablePresentModes;
	};
}