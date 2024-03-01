#include "vulkan_device.h"
#include "vulkan_surface.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace vulkan;

VulkanDevice::VulkanDevice(VulkanInstance* instance, rhi::GpuPreference gpuPreference) : _instance(instance)
{
	LOG_INFO("VulkanDevice::VulkanDevice(): Start initing")
	
	pick_device(gpuPreference);
	create_allocator();

	LOG_INFO("VulkanDevice::VulkanDevice(): Finish initing")
}

void VulkanDevice::find_present_queue(VulkanSurface* surface)
{
	uint32_t presentFamily = VK_QUEUE_FAMILY_IGNORED;
	uint32_t queueFamilyCount = (uint32_t)_queueFamiliesProperties.size();
	
	for (uint32_t i = 0; i != queueFamilyCount; ++i)
	{
		const auto& queueFamilyProperties = _queueFamiliesProperties[i].queueFamilyProperties;
		VkBool32 presentSupport = VK_FALSE;
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface->get_handle(), &presentSupport));

		if (presentFamily == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && presentSupport)
		{
			presentFamily = i;
			break;
		}
	}
	
	_presentQueue = std::make_unique<VulkanQueue>(this, presentFamily, rhi::QueueType::GRAPHICS, false);
}

void VulkanDevice::fill_gpu_properties(rhi::GpuProperties& gpuProperties)
{
	gpuProperties.vendorID = _properties2.properties.vendorID;
	gpuProperties.deviceID = _properties2.properties.deviceID;
	gpuProperties.gpuName = _properties2.properties.deviceName;
	gpuProperties.driverDescription = _properties1_2.driverName;
	if (_properties1_2.driverInfo[0] != '\0')
	{
		gpuProperties.driverDescription += std::string(": ") + _properties1_2.driverInfo;
	}

	switch (_properties2.properties.deviceType)
	{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			gpuProperties.gpuType = rhi::GpuType::INTEGRATED;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			gpuProperties.gpuType = rhi::GpuType::DISCRETE;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			gpuProperties.gpuType = rhi::GpuType::VIRTUAL;
			break;
		default:
			gpuProperties.gpuType = rhi::GpuType::OTHER;
			break;
	}

	if (_features2.features.tessellationShader == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has tessellation shader capability")
		gpuProperties.capabilities |= rhi::GpuCapability::TESSELLATION;
	}
	if (
		_rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
		_rayQueryFeatures.rayQuery == VK_TRUE &&
		_accelerationStructureFeatures.accelerationStructure == VK_TRUE &&
		_features1_2.bufferDeviceAddress == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has ray tracing capability")
		gpuProperties.capabilities |= rhi::GpuCapability::RAY_TRACING;
		gpuProperties.shaderIdentifierSize = _rayTracingPipelineProperties.shaderGroupHandleSize;
		gpuProperties.accelerationStructureInstanceSize = sizeof(VkAccelerationStructureInstanceKHR);
	}
	if (_meshShaderFeatures.meshShader == VK_TRUE && _meshShaderFeatures.taskShader == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has task shader and mesh shader capabilities")
		gpuProperties.capabilities |= rhi::GpuCapability::MESH_SHADER;
	}
	if (_fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has variable rate shading capability")
		gpuProperties.capabilities |= rhi::GpuCapability::VARIABLE_RATE_SHADING;
	}
	if (_fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has varialbe rate shading tier 2 capability")
		gpuProperties.capabilities |= rhi::GpuCapability::VARIABLE_RATE_SHADING_TIER2;
	}
	if (_fragmentShadingRateProperties.fragmentShadingRateWithFragmentShaderInterlock == VK_TRUE)
	{
		LOG_INFO("Vulkan: GPU has fragment shader intelock capability")
		gpuProperties.capabilities |= rhi::GpuCapability::FRAGMENT_SHADER_INTERLOCK;
	}
	if (_features2.features.sparseBinding == VK_TRUE && _features2.features.sparseResidencyAliased == VK_TRUE)
	{
		if (_properties2.properties.sparseProperties.residencyNonResidentStrict == VK_TRUE)
		{
			LOG_INFO("Vulkan: GPU has sparse null mapping capability")
			gpuProperties.capabilities |= rhi::GpuCapability::SPARSE_NULL_MAPPING;
		}
		if (_features2.features.sparseResidencyBuffer == VK_TRUE)
		{
			LOG_INFO("Vulkan: GPU has sparse buffer capability")
			gpuProperties.capabilities |= rhi::GpuCapability::SPARSE_BUFFER;
		}
		if (_features2.features.sparseResidencyImage2D == VK_TRUE)
		{
			LOG_INFO("Vulkan: GPU has sparse texture2D capability")
			gpuProperties.capabilities |= rhi::GpuCapability::SPARSE_TEXTURE2D;
		}
		if (_features2.features.sparseResidencyImage3D == VK_TRUE)
		{
			LOG_INFO("Vulkan: GPU has sparse texture3D capability")
			gpuProperties.capabilities |= rhi::GpuCapability::SPARSE_TEXTURE3D;
		}
		LOG_INFO("Vulkan: GPU has sparse tile pool capability")
		gpuProperties.capabilities |= rhi::GpuCapability::SPARSE_TILE_POOL;
	}
	const VkPhysicalDeviceMemoryProperties& memoryProperties = _memoryProperties2.memoryProperties;
	for (uint32_t i = 0; i != memoryProperties.memoryHeapCount; ++i)
	{
		if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			for (uint32_t j = 0; j != memoryProperties.memoryTypeCount; ++j)
			{
				if (memoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
					memoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
				{
					LOG_INFO("Vulkan: GPU has cache coherent UMA capability")
					gpuProperties.capabilities |= rhi::GpuCapability::CACHE_COHERENT_UMA;
					break;
				}
			}
		}
		if (has_flag(gpuProperties.capabilities, rhi::GpuCapability::CACHE_COHERENT_UMA))
		{
			break;
		}
	}
}

void VulkanDevice::cleanup()
{
	vmaDestroyAllocator(_allocator);
	vkDestroyDevice(_device, nullptr);
}

void VulkanDevice::pick_device(rhi::GpuPreference preference)
{
	uint32_t deviceCount;
	VK_CHECK(vkEnumeratePhysicalDevices(_instance->get_handle(), &deviceCount, nullptr));
	if (deviceCount == 0)
	{
		LOG_FATAL("VulkanDevice::pick_physical_device(): Device count is 0")
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(_instance->get_handle(), &deviceCount, devices.data()));

	std::vector<const char*> enabledExtensions;
	
	for (auto& device : devices)
	{
		if (!validate_physical_device(device, enabledExtensions))
			continue;

		bool priority = _properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		
		if (preference == rhi::GpuPreference::INTEGRATED)
		{
			priority = _properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		}
		if (priority || _physicalDevice == VK_NULL_HANDLE)
		{
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE)
	{
		LOG_FATAL("VulkanDevice::pick_physical_device(): Failed to pick physical device")
	}

	assert(_properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE);
	assert(_features2.features.imageCubeArray == VK_TRUE);
	assert(_features2.features.geometryShader == VK_TRUE);
	assert(_features2.features.textureCompressionBC == VK_TRUE);
	assert(_features2.features.samplerAnisotropy == VK_TRUE);
	assert(_features2.features.multiViewport == VK_TRUE);
	assert(_features2.features.pipelineStatisticsQuery == VK_TRUE);
	assert(_features2.features.fragmentStoresAndAtomics == VK_TRUE);
	assert(_features1_2.runtimeDescriptorArray == VK_TRUE);
	assert(_features1_2.descriptorIndexing == VK_TRUE);
	assert(_features1_3.synchronization2 == VK_TRUE);
	assert(_features1_3.dynamicRendering == VK_TRUE);
	assert(_features1_3.maintenance4 == VK_TRUE);

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &queueFamilyCount, nullptr);
	_queueFamiliesProperties.resize(queueFamilyCount);
	for (uint32_t i = 0; i != queueFamilyCount; ++i)
	{
		_queueFamiliesProperties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		_queueFamiliesProperties[i].pNext = nullptr;	// will use for video queue in the future
	}
	vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &queueFamilyCount, _queueFamiliesProperties.data());

	uint32_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
	uint32_t computeFamily = VK_QUEUE_FAMILY_IGNORED;
	uint32_t transferFamily = VK_QUEUE_FAMILY_IGNORED;
	
	bool doesGraphicsQueueSupportSparseBinding = false;
	bool doesComputeQueueSupportSparseBinding = false;
	bool doesTransferQueueSupportSparseBinding = false;

	for (uint32_t i = 0; i != queueFamilyCount; ++i)
	{
		const auto& queueFamilyProperties = _queueFamiliesProperties[i].queueFamilyProperties;
		if (graphicsFamily == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsFamily = i;
			if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				doesGraphicsQueueSupportSparseBinding = true;
			}
		}
		if (computeFamily == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeFamily = i;
		}
		if (transferFamily == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			transferFamily = i;
		}
	}

	for (uint32_t i = 0; i != queueFamilyCount; ++i)
	{
		const auto& queueFamilyProperties = _queueFamiliesProperties[i].queueFamilyProperties;
		
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT &&
			!(queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			!(queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			transferFamily = i;
			if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				doesTransferQueueSupportSparseBinding = true;
			}
		}

		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT &&
			!(queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			computeFamily = i;
			if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				doesComputeQueueSupportSparseBinding = true;
			}
		}
	}

	std::vector<uint32_t> uniqueQueueFamilies = { graphicsFamily, computeFamily, transferFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	const float queuePriority = 1.0f;
	for (const auto queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo& createInfo = queueCreateInfos.emplace_back();
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &queuePriority;
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.pNext = &_features2;
	deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device));

	volkLoadDevice(_device);

	_graphicsQueue = std::make_unique<VulkanQueue>(this, graphicsFamily, rhi::QueueType::GRAPHICS, doesGraphicsQueueSupportSparseBinding);
	_computeQueue = std::make_unique<VulkanQueue>(this, computeFamily, rhi::QueueType::COMPUTE, doesComputeQueueSupportSparseBinding);
	_transferQueue = std::make_unique<VulkanQueue>(this, transferFamily, rhi::QueueType::TRANSFER, doesTransferQueueSupportSparseBinding);
}

template<typename T>
void add_to_chain(void*** chain, T* obj)
{
	**chain = obj;
	*chain = &obj->pNext;
}

bool VulkanDevice::validate_physical_device(VkPhysicalDevice device, std::vector<const char*>& enabledExtensions)
{
	static const std::vector<const char*> requiredExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	uint32_t extensionCount;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

	for (auto& requiredExtension : requiredExtensions)
	{
		if (!is_extension_supported(requiredExtension, availableExtensions))
		{
			return false;
		}
	}

	_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	_features1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	_features1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	_features1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	_features2.pNext = &_features1_1;
	_features1_1.pNext = &_features1_2;
	_features1_2.pNext = &_features1_3;
	void** featuresChain = &_features1_3.pNext;

	_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	_properties1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	_properties1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
	_properties1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
	_properties2.pNext = &_properties1_1;
	_properties1_1.pNext = &_properties1_2;
	_properties1_2.pNext = &_properties1_3;
	void** propertiesChain = &_properties1_3.pNext;
	
	_multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
	add_to_chain(&propertiesChain, &_multiviewProperties);

	_samplerMinMaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
	add_to_chain(&propertiesChain, &_samplerMinMaxProperties);

	enabledExtensions = requiredExtensions;

	if (is_extension_supported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, availableExtensions))
	{
		enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		assert(is_extension_supported(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, availableExtensions));
		enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

		_accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		add_to_chain(&featuresChain, &_accelerationStructureFeatures);

		_accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		add_to_chain(&propertiesChain, &_accelerationStructureProperties);

		if (is_extension_supported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, availableExtensions))
		{
			enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			enabledExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

			_rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			add_to_chain(&featuresChain, &_rayTracingPipelineFeatures);

			_rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			add_to_chain(&propertiesChain, &_rayTracingPipelineProperties);
		}

		if (is_extension_supported(VK_KHR_RAY_QUERY_EXTENSION_NAME, availableExtensions))
		{
			enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

			_rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
			add_to_chain(&featuresChain, &_rayQueryFeatures);
		}
	}
	
	if (is_extension_supported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, availableExtensions))
	{
		enabledExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

		_fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
		add_to_chain(&featuresChain, &_fragmentShadingRateFeatures);

		_fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
		add_to_chain(&propertiesChain, &_fragmentShadingRateProperties);
	}

	if (is_extension_supported(VK_EXT_MESH_SHADER_EXTENSION_NAME, availableExtensions))
	{
		enabledExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

		_meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
		add_to_chain(&featuresChain, &_meshShaderFeatures);

		_meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
		add_to_chain(&propertiesChain, &_meshShaderProperties);
	}

	*propertiesChain = nullptr;
	*featuresChain = nullptr;
	vkGetPhysicalDeviceProperties2(device, &_properties2);
	vkGetPhysicalDeviceFeatures2(device, &_features2);
	
	_memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	vkGetPhysicalDeviceMemoryProperties2(device, &_memoryProperties2);

	return true;
}

bool VulkanDevice::is_extension_supported(const char* requiredExtension, const std::vector<VkExtensionProperties>& availableExtensions)
{
	for (auto& availableExtension : availableExtensions)
	{
		if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
		{
			LOG_INFO("Vulkan: {} is supported", requiredExtension)
			return true;
		}
	}
	return false;
}

void VulkanDevice::create_allocator()
{
	VmaVulkanFunctions vmaVulkanFunctions{};
	vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vmaVulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaVulkanFunctions.vkAllocateMemory = vkAllocateMemory;
	vmaVulkanFunctions.vkFreeMemory = vkFreeMemory;
	vmaVulkanFunctions.vkMapMemory = vkMapMemory;
	vmaVulkanFunctions.vkUnmapMemory = vkUnmapMemory;
	vmaVulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vmaVulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vmaVulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vmaVulkanFunctions.vkBindImageMemory = vkBindImageMemory;
	vmaVulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vmaVulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vmaVulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vmaVulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vmaVulkanFunctions.vkCreateImage = vkCreateImage;
	vmaVulkanFunctions.vkDestroyImage = vkDestroyImage;
	vmaVulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vmaVulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
	vmaVulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.physicalDevice = _physicalDevice;
	allocatorCreateInfo.device = _device;
	allocatorCreateInfo.instance = _instance->get_handle();
	
	if (_features1_2.bufferDeviceAddress)
	{
		allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaVulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
		vmaVulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
	}

	allocatorCreateInfo.pVulkanFunctions = &vmaVulkanFunctions;

	VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &_allocator));
}
