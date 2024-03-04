#include "vulkan_instance.h"
#include "vulkan_common.h"

using namespace ad_astris;
using namespace vulkan;

bool validate_layers(const std::vector<const char*>& requiredLayers, const std::vector<VkLayerProperties>& availableLayers)
{
	for (auto& requiredLayer : requiredLayers)
	{
		bool found = false;
		for (auto& availableLayer : availableLayers)
		{
			if (strcmp(requiredLayer, availableLayer.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}
	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LOG_WARNING("Vulkan: {}", callbackData->pMessage)
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LOG_ERROR("Vulkan: {}", callbackData->pMessage)
	}

	return VK_FALSE;
}

VulkanInstance::VulkanInstance(rhi::ValidationMode validationMode)
{
	VK_CHECK(volkInitialize());
	
	uint32_t instanceLayerCount;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
	std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));
	VkApplicationInfo appInfo{};

	uint32_t extensionCount;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
	std::vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));

	std::vector<const char*> instanceLayers;
	std::vector<const char*> instanceExtensions;

	bool canUseDebugUtils = false;

	for (auto& availableExtension : availableInstanceExtensions)
	{
		if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
		{
			canUseDebugUtils = true;
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		else if (strcmp(availableExtension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
		{
			instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}
		else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
		{
			instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
		}
		else if (strcmp(availableExtension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		else if (strcmp(availableExtension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		}
#endif
		else if (strcmp(availableExtension.extensionName, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME) == 0)
		{
			instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
		}
	}

	if (validationMode != rhi::ValidationMode::DISABLED)
	{
		// Took from https://github.com/turanszkij/WickedEngine/blob/1c49e4984a7f63be44d06d7b7c0949386d531bd8/WickedEngine/wiGraphicsDevice_Vulkan.cpp
		static const std::vector<const char*> validationLayersPriority[] =
		{
			{ "VK_LAYER_KHRONOS_validation" },
			{ "VK_LAYER_LUNARG_standard_validation" },
			{
				"VK_LAYER_GOOGLE_threading",
				"VK_LAYER_LUNARG_parameter_validation",
				"VK_LAYER_LUNARG_object_tracker",
				"VK_LAYER_LUNARG_core_validation",
				"VK_LAYER_GOOGLE_unique_objects",
			},
			{ "VK_LAYER_LUNARG_core_validation" }	
		};

		for (auto& validationLayers : validationLayersPriority)
		{
			if (validate_layers(validationLayers, availableInstanceLayers))
			{
				for (auto& layer : validationLayers)
				{
					instanceLayers.push_back(layer);
				}
				break;
			}
		}
	}
	
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "AdAstris Engine Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "AdAstris Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount = instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
	debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	if (validationMode != rhi::ValidationMode::DISABLED && canUseDebugUtils)
	{
		debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugUtilsCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		if (validationMode == rhi::ValidationMode::VERBOSE)
		{
			debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
			debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		}

		debugUtilsCreateInfo.pfnUserCallback = debug_utils_messenger_callback;
		instanceCreateInfo.pNext = &debugUtilsCreateInfo;
	}
	
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));

	volkLoadInstanceOnly(_instance);

	if (validationMode != rhi::ValidationMode::DISABLED && canUseDebugUtils)
	{
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsCreateInfo, nullptr, &_debugUtilsMessenger));
	}
}

void VulkanInstance::cleanup()
{
	vkDestroyDebugUtilsMessengerEXT(_instance, _debugUtilsMessenger, nullptr);
	vkDestroyInstance(_instance, nullptr);
}
