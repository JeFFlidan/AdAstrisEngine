#pragma once

#include "vulkan_api.h"
#include "rhi/resources.h"

namespace ad_astris::vulkan
{
	class VulkanInstance
	{
		public:
			VulkanInstance(rhi::ValidationMode validationMode);

			void cleanup();

			VkInstance get_handle() const { return _instance; }
			VkDebugUtilsMessengerEXT get_debug_utils_messenger() const { return _debugUtilsMessenger; }
		
		private:
			VkInstance _instance{ VK_NULL_HANDLE };
			VkDebugUtilsMessengerEXT _debugUtilsMessenger{ VK_NULL_HANDLE };
	};
}