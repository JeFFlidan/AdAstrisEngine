#include "vulkan_device.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
	#include <vulkan/vulkan_win32.h>
#endif

#include "vulkan_queue.h"
#include "profiler/logger.h"
#include "vulkan_common.h"
#include <algorithm>

using namespace ad_astris;

vulkan::VulkanDevice::VulkanDevice(vkb::Instance& instance, acore::IWindow* window)
{
	LOG_INFO("Start initing Device class (Vulkan)")

	create_surface(instance, window);
	vkb::PhysicalDevice vkbPhysDevice = pick_physical_device(instance);
	vkb::Device vkbDevice = pick_device(vkbPhysDevice);

	QueueData graphicsQueue;
	QueueData computeQueue;
	QueueData presentQueue;
	QueueData transferQueue;

	graphicsQueue.queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueue.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	graphicsQueue.queueType = rhi::QueueType::GRAPHICS;
	_graphicsQueue = new VulkanQueue(graphicsQueue);
	computeQueue.queue = vkbDevice.get_queue(vkb::QueueType::compute).value();
	computeQueue.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::compute).value();
	computeQueue.queueType = rhi::QueueType::COMPUTE;
	_computeQueue = new VulkanQueue(computeQueue);
	// Maybe I'll remove present queue
	presentQueue.queue = vkbDevice.get_queue(vkb::QueueType::present).value();
	presentQueue.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::present).value();
	presentQueue.queueType = rhi::QueueType::GRAPHICS;
	_presentQueue = new VulkanQueue(presentQueue);
	transferQueue.queue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
	transferQueue.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
	transferQueue.queueType = rhi::QueueType::TRANSFER;
	_transferQueue = new VulkanQueue(transferQueue);

	_physicalDevice = vkbPhysDevice.physical_device;
	_device = vkbDevice.device;
	get_properties();

	LOG_INFO("Finished initing Device class (Vulkan)")
}

vulkan::VulkanQueue* vulkan::VulkanDevice::get_graphics_queue() { return _graphicsQueue; }
vulkan::VulkanQueue* vulkan::VulkanDevice::get_present_queue() { return _presentQueue; }
vulkan::VulkanQueue* vulkan::VulkanDevice::get_compute_queue() { return _computeQueue; }
vulkan::VulkanQueue* vulkan::VulkanDevice::get_transfer_queue() { return _transferQueue; }

void vulkan::VulkanDevice::cleanup()
{
	delete _graphicsQueue;
	delete _presentQueue;
	delete _computeQueue;
	delete _transferQueue;
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyDevice(_device, nullptr);
}

void vulkan::VulkanDevice::create_surface(VkInstance instance, acore::IWindow* window)
{
	LOG_INFO("VulkanDevice::VulkanDevice(): Start creating surface")
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = window->get_hWnd();
	createInfo.hinstance = GetModuleHandle(nullptr);
	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &_surface) != VK_SUCCESS)
		LOG_FATAL("VulkanDevice::VulkanDevice(): Failed to create vulkan surface")
#endif
	LOG_INFO("VulkanDevice::VulkanDevice(): Finish creating surface")
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
	features1_2.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
	features1_2.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
	features1_2.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
	features1_2.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
	features1_2.samplerFilterMinmax = VK_TRUE;
	features1_2.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	
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

	VkPhysicalDeviceMultiviewProperties multiviewProperties{};
	multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &multiviewProperties;
	vkGetPhysicalDeviceProperties2(physicalDevice.physical_device, &properties2);
	_maxMultiviewView = multiviewProperties.maxMultiviewViewCount;
	LOG_INFO("Max multiview view: {}", _maxMultiviewView)
	
	VkPhysicalDeviceMultiviewFeatures multiViewFeatures{};
	multiViewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
	multiViewFeatures.multiview = VK_TRUE;

	deviceBuilder.add_pNext(&multiViewFeatures);
	
	VkPhysicalDeviceMaintenance4Features maintenance4Features{};
	maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
	maintenance4Features.maintenance4 = VK_TRUE;
	deviceBuilder.add_pNext(&maintenance4Features);
	
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

void vulkan::VulkanDevice::get_properties()
{
	_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	_properties1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	_properties1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
	_properties1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
	_properties2.pNext = &_properties1_2;
	//_properties1_1.pNext = &_properties1_2;
	//_properties1_2.pNext = &_properties1_3;
	//_properties1_3.pNext = nullptr;
	vkGetPhysicalDeviceProperties2(_physicalDevice, &_properties2);
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
