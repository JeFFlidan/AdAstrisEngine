#pragma once

#include "vulkan_api.h"
#include "vulkan_queue.h"
#include "vulkan_instance.h"
#include "profiler/logger.h"
#include <vector>

namespace ad_astris::vulkan
{
	class VulkanInstance;
	class VulkanSurface;

	class VulkanDevice
	{
		public:
			VulkanDevice(VulkanInstance* instance, rhi::GpuPreference gpuPreference);

			void find_present_queue(VulkanSurface* surface);
			void fill_gpu_properties(rhi::GpuProperties& gpuProperties);
			void cleanup();
		
			VulkanInstance* get_instance() const { return _instance; }
			VkPhysicalDevice get_physical_device() const { return _physicalDevice; }
			VkDevice get_device() const { return _device; }
			VmaAllocator get_allocator() const { return _allocator; }

			VulkanQueue* get_graphics_queue() const { return _graphicsQueue.get(); }
			VulkanQueue* get_compute_queue() const { return _computeQueue.get(); }
			VulkanQueue* get_transfer_queue() const { return _transferQueue.get(); }
			VulkanQueue* get_present_queue() const { return _presentQueue.get(); }

			const VkPhysicalDeviceProperties2& get_physical_device_properties() const
			{
				return _properties2;
			}

			const VkPhysicalDeviceVulkan11Properties& get_physical_device_vulkan_1_1_properties() const
			{
				return _properties1_1;
			}

			const VkPhysicalDeviceVulkan12Properties& get_physical_device_vulkan_1_2_properties() const
			{
				return _properties1_2;
			}

			const VkPhysicalDeviceVulkan13Properties& get_physical_device_vulkan_1_3_properties() const
			{
				return _properties1_3;
			}

			const VkPhysicalDeviceMemoryProperties2& get_memory_properties() const
			{
				return _memoryProperties2;
			}

			const VkPhysicalDeviceMultiviewProperties& get_multiview_properties() const
			{
				return _multiviewProperties;
			}

			const VkPhysicalDeviceSamplerFilterMinmaxProperties& get_sampler_minmax_properties() const
			{
				return _samplerMinMaxProperties;
			}

			const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const
			{
				return _accelerationStructureProperties;
			}

			const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const
			{
				return _rayTracingPipelineProperties;
			}

			const VkPhysicalDeviceMeshShaderPropertiesEXT& get_mesh_shader_properties() const
			{
				return _meshShaderProperties;
			}

			const VkPhysicalDeviceFragmentShadingRatePropertiesKHR& get_fragment_shading_rate_properties() const
			{
				return _fragmentShadingRateProperties;
			}
		
		private:
			VulkanInstance* _instance{ nullptr };
			VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
			VkDevice _device{ VK_NULL_HANDLE };
			VmaAllocator _allocator{ VK_NULL_HANDLE };
		
			VkPhysicalDeviceProperties2 _properties2{};
			VkPhysicalDeviceVulkan11Properties _properties1_1{};
			VkPhysicalDeviceVulkan12Properties _properties1_2{};
			VkPhysicalDeviceVulkan13Properties _properties1_3{};
			VkPhysicalDeviceMemoryProperties2 _memoryProperties2{};
			VkPhysicalDeviceMultiviewProperties _multiviewProperties{};
			VkPhysicalDeviceSamplerFilterMinmaxProperties _samplerMinMaxProperties{};
			VkPhysicalDeviceAccelerationStructurePropertiesKHR _accelerationStructureProperties{};
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rayTracingPipelineProperties{};
			VkPhysicalDeviceMeshShaderPropertiesEXT _meshShaderProperties{};
			VkPhysicalDeviceFragmentShadingRatePropertiesKHR _fragmentShadingRateProperties{};

			VkPhysicalDeviceFeatures2 _features2{};
			VkPhysicalDeviceVulkan11Features _features1_1{};
			VkPhysicalDeviceVulkan12Features _features1_2{};
			VkPhysicalDeviceVulkan13Features _features1_3{};
			VkPhysicalDeviceMultiviewFeatures _multiviewFeatures{};
			VkPhysicalDeviceShaderDrawParametersFeatures _shaderDrawParametersFeatures{};
			VkPhysicalDeviceAccelerationStructureFeaturesKHR _accelerationStructureFeatures{};
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR _rayTracingPipelineFeatures{};
			VkPhysicalDeviceRayQueryFeaturesKHR _rayQueryFeatures{};
			VkPhysicalDeviceFragmentShadingRateFeaturesKHR _fragmentShadingRateFeatures{};
			VkPhysicalDeviceMeshShaderFeaturesEXT _meshShaderFeatures{};

			std::vector<VkQueueFamilyProperties2> _queueFamiliesProperties;

			std::unique_ptr<VulkanQueue> _graphicsQueue{ nullptr };
			std::unique_ptr<VulkanQueue> _computeQueue{ nullptr };
			std::unique_ptr<VulkanQueue> _transferQueue{ nullptr };
			std::unique_ptr<VulkanQueue> _presentQueue{ nullptr };
		
			void pick_device(rhi::GpuPreference preference);
			bool validate_physical_device(VkPhysicalDevice device, std::vector<const char*>& enabledExtensions);
			bool is_extension_supported(const char* requiredExtension, const std::vector<VkExtensionProperties>& availableExtensions);
			void create_allocator();
	};
}