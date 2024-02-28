#include "vulkan_surface.h"
#include "vulkan_common.h"
#include "profiler/logger.h"
#include "core/platform.h"

using namespace ad_astris;
using namespace vulkan;

VulkanSurface::VulkanSurface(VulkanInstance* instance, VulkanDevice* device, acore::IWindow* window)
{
	LOG_INFO("VulkanDevice::VulkanDevice(): Start creating surface")
#ifdef _WIN32
VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = window->get_hWnd();
	createInfo.hinstance = GetModuleHandle(nullptr);
	if (vkCreateWin32SurfaceKHR(instance->get_handle(), &createInfo, nullptr, &_surface) != VK_SUCCESS)
		LOG_FATAL("VulkanDevice::VulkanDevice(): Failed to create vulkan surface")
#endif

	VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
	surfaceInfo.surface = _surface;

	uint32_t formatCount;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormats2KHR(device->get_physical_device(), &surfaceInfo, &formatCount, nullptr));
	_availableFormats.resize(formatCount);
	for (auto& format : _availableFormats)
	{
		format.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
	}
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormats2KHR(device->get_physical_device(), &surfaceInfo, &formatCount, _availableFormats.data()));

	uint32_t presentModeCount;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModes2EXT(device->get_physical_device(), &surfaceInfo, &presentModeCount, nullptr));
	_availablePresentModes.resize(presentModeCount);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModes2EXT(device->get_physical_device(), &surfaceInfo, &presentModeCount, _availablePresentModes.data()));
	
	LOG_INFO("VulkanDevice::VulkanDevice(): Finish creating surface")
}

void VulkanSurface::cleanup(VulkanInstance* instance)
{
	vkDestroySurfaceKHR(instance->get_handle(), _surface, nullptr);
}

VkSurfaceCapabilitiesKHR VulkanSurface::get_capabilities(VulkanDevice* device) const
{
	VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
	surfaceInfo.surface = _surface;

	VkSurfaceCapabilities2KHR capabilities2;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilities2KHR(device->get_physical_device(), &surfaceInfo, &capabilities2));
	return capabilities2.surfaceCapabilities;
}
