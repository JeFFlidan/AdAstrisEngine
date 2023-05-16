#pragma once

#include "profiler/logger.h"
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vector>
#include <string>

namespace ad_astris::vulkan
{
	struct OptionalExtensions
	{
		uint8_t meshShader{ 0 };
	};

	class VulkanQueue;

	class VulkanDevice
	{
		public:
			VulkanDevice(vkb::Instance& instance, void* window);
			~VulkanDevice();

			// Maybe I shouldn't make those variables private
			VkSurfaceKHR get_surface() { return _surface; }
			VkPhysicalDevice get_physical_device() { return _physicalDevice; }
			VkDevice get_device() { return _device; }

			uint32_t get_max_multiview_view_count() { return _maxMultiviewView; }

			VulkanQueue* get_graphics_queue() { return _graphicsQueue; }
			VulkanQueue* get_present_queue() { return _presentQueue; }
			VulkanQueue* get_compute_queue() { return _computeQueue; }
			VulkanQueue* get_transfer_queue() { return _transferQueue; }
		
		private:
			VkSurfaceKHR _surface{ VK_NULL_HANDLE };
			VkInstance _instance{ VK_NULL_HANDLE };
			VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
			VkDevice _device{ VK_NULL_HANDLE };

			std::vector<std::string> _requieredExtensions{
				"VK_EXT_descriptor_indexing",
				"VK_EXT_sampler_filter_minmax",
				"VK_KHR_multiview",
				"VK_KHR_maintenance4" };
			std::vector<std::string> _optionalExtensions{
				"VK_EXT_mesh_shader" };
			OptionalExtensions _isOptionalExtensionsEnabled{};
			uint32_t _maxMultiviewView;

			VulkanQueue* _graphicsQueue{ nullptr };
			VulkanQueue* _presentQueue{ nullptr };
			VulkanQueue* _computeQueue{ nullptr };
			VulkanQueue* _transferQueue{ nullptr };

			// I use SDL window for Windows but it will be replaced
			/** Creates Vulkan surface.
			 @param instance should be valid Vulkan instance
			 @param window should be pointer to the window: SDL for Linux or WinApi for Windows.
			 */
			void create_surface(VkInstance instance, void* window);
			vkb::PhysicalDevice pick_physical_device(vkb::Instance& instance);
			vkb::Device pick_device(vkb::PhysicalDevice& physicalDevice);
			bool check_needed_extensions(std::vector<std::string>& supportedExt, const std::string& extName);
			void get_supported_extensions_and_features(
				vkb::Instance& instance,
				std::vector<std::string>& extensions,
				VkPhysicalDeviceFeatures& features);
			void set_feature(VkBool32 supported, VkBool32& feature, std::string featureName);
			void set_optional_extension(std::string& ext);
	};
}