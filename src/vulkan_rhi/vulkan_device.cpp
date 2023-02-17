#include "vulkan_device.h"
#include "profiler/logger.h"
#include "vulkan_common.h"
#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <algorithm>

using namespace ad_astris;

void vulkan::VulkanDevice::init(vkb::Instance& instance, void* window)
{
	LOG_INFO("Start initing Device class (Vulkan)")

	create_surface(instance, window);
	if (_surface == VK_NULL_HANDLE)
		LOG_ERROR("Null handle")
	vkb::PhysicalDevice vkbPhysDevice = pick_physical_device(instance);
	vkb::Device vkbDevice = pick_device(vkbPhysDevice);

	_graphicsQueue = new QueueData();
	_computeQueue = new QueueData();
	_presentQueue = new QueueData();
	_transferQueue = new QueueData();

	_graphicsQueue->queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueue->queueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	_computeQueue->queue = vkbDevice.get_queue(vkb::QueueType::compute).value();
	_computeQueue->queueFamily = vkbDevice.get_queue_index(vkb::QueueType::compute).value();
	_presentQueue->queue = vkbDevice.get_queue(vkb::QueueType::present).value();
	_presentQueue->queueFamily = vkbDevice.get_queue_index(vkb::QueueType::present).value();
	_transferQueue->queue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
	_transferQueue->queueFamily = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();

	_physicalDevice = vkbPhysDevice.physical_device;
	_device = vkbDevice.device;

	LOG_INFO("Finish initing Device class (Vulkan)")
}

void vulkan::VulkanDevice::cleanup()
{
	delete _graphicsQueue;
	delete _presentQueue;
	delete _computeQueue;
	delete _transferQueue;
}

void vulkan::VulkanDevice::create_surface(VkInstance instance, void* window)
{
	LOG_INFO("Start creating surface")
#ifdef _WIN32
	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window);
	SDL_bool valid = SDL_Vulkan_CreateSurface(sdlWindow, instance, &_surface);
	if (!valid)
		LOG_FATAL("Failed to create Vulkan Surface for SDL window")
#endif
	LOG_INFO("Finish creating surface")
}

vkb::PhysicalDevice vulkan::VulkanDevice::pick_physical_device(vkb::Instance& instance)
{
	LOG_INFO("Start picking physical device")
	
	std::vector<std::string> supportedExtensions;
	VkPhysicalDeviceFeatures supportedFeatures;

	get_supported_extensions_and_features(instance, supportedExtensions, supportedFeatures);
	
	vkb::PhysicalDeviceSelector physSelector{ instance };
	physSelector.set_minimum_version(1, 2);
	physSelector.set_surface(_surface);

	for (auto& ext : _requieredExtensions)
	{
		if (check_needed_extensions(supportedExtensions, ext))
			physSelector.add_required_extension(ext.c_str());
		else
			LOG_FATAL("Your GPU doesn't support required Vulkan extension {}", ext.c_str())
	}
	
	for (auto& ext : _optionalExtensions)
	{
		if (check_needed_extensions(supportedExtensions, ext))
		{
			physSelector.add_required_extension(ext.c_str());
			set_optional_extension(ext);
		}
		else
			LOG_WARNING("Your GPU doesn't support optional Vulkan extenstion {}", ext.c_str())
	}

	VkPhysicalDeviceVulkan12Features features1_2{};
	features1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features1_2.runtimeDescriptorArray = VK_TRUE;
	features1_2.descriptorIndexing = VK_TRUE;
	features1_2.descriptorBindingPartiallyBound = VK_TRUE;
	features1_2.descriptorBindingVariableDescriptorCount = VK_TRUE;
	features1_2.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	features1_2.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	features1_2.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	features1_2.samplerFilterMinmax = VK_TRUE;
	
	VkPhysicalDeviceFeatures enabledFeatures{};
	set_feature(supportedFeatures.samplerAnisotropy, enabledFeatures.samplerAnisotropy, "samplerAnisotropy");
	set_feature(supportedFeatures.fragmentStoresAndAtomics, enabledFeatures.fragmentStoresAndAtomics, "fragmentStoresAndAtomics");
	
	physSelector.set_required_features(enabledFeatures);
	physSelector.set_required_features_12(features1_2);

	_requieredExtensions.clear();
	_optionalExtensions.clear();

	LOG_INFO("Finish picking physical device")
	
	return physSelector.select().value();
}

vkb::Device vulkan::VulkanDevice::pick_device(vkb::PhysicalDevice& physicalDevice)
{
	LOG_INFO("Start picking logical device")
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures{};
	shaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	shaderDrawParametersFeatures.pNext = nullptr;
	shaderDrawParametersFeatures.shaderDrawParameters = VK_TRUE;

	deviceBuilder.add_pNext(&shaderDrawParametersFeatures);

	VkPhysicalDeviceMultiviewFeatures multiViewFeatures{};
	multiViewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
	multiViewFeatures.multiview = VK_TRUE;
	
	deviceBuilder.add_pNext(&multiViewFeatures);
	
	LOG_INFO("Finish picking logical device")
	return deviceBuilder.build().value();
}

bool vulkan::VulkanDevice::check_needed_extensions(std::vector<std::string>& supportedExt, const std::string& extName)
{
	return std::find(supportedExt.begin(), supportedExt.end(), extName) != supportedExt.end();
}

void vulkan::VulkanDevice::get_supported_extensions_and_features(
	vkb::Instance& instance,
	std::vector<std::string>& extensions,
	VkPhysicalDeviceFeatures& features)
{
	LOG_INFO("Start getting supported extensions and features")
	vkb::PhysicalDeviceSelector selector{ instance };
	vkb::PhysicalDevice physDevice = selector
		.set_minimum_version(1, 2)
		.set_surface(_surface)
		.select()
		.value();
	
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> supportedExtension(count);
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &count, supportedExtension.data());

	for (auto& extension : supportedExtension)
		extensions.push_back(extension.extensionName);

	vkGetPhysicalDeviceFeatures(physDevice, &features);
	
	LOG_INFO("Finish getting supported extensions and features")
}

void vulkan::VulkanDevice::set_feature(VkBool32 supported, VkBool32& feature, std::string featureName)
{
	if (supported)
		feature = VK_TRUE;
	else
		LOG_FATAL("Your GPU doesn't support required Vulkan feature {}", featureName.c_str())
}

void vulkan::VulkanDevice::set_optional_extension(std::string& ext)
{
	if (ext == "VK_EXT_mesh_shader")
		_isOptionalExtensionsEnabled.meshShader = 1;
}
