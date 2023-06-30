#pragma once

#include "core/module.h"
#include "rhi/resources.h"
#include "rhi/engine_rhi.h"

// FOR TESTS
#include <vulkan/vulkan.h>

namespace ad_astris
{
	namespace vulkan
	{
		class VulkanRHI;
		class VulkanDevice;
		class VulkanQueue;
		class VulkanCommandManager;
	}
	
	class IVulkanRHIModule : public IModule
	{
		public:
			virtual rhi::IEngineRHI* create_vulkan_rhi() = 0;
	};

	// FOR TESTS
	class IVulkanQueue
	{
		public:
			virtual void submit(vulkan::VulkanCommandManager& cmdManager) = 0;
			virtual void present() = 0;

			virtual VkQueue get_queue() = 0;
			virtual uint32_t get_family() = 0;
			virtual rhi::QueueType get_queue_type() = 0;
			virtual uint32_t get_submission_counter() = 0;
	};

	class IVulkanPipeline
	{
		public:
			virtual VkPipeline get_handle() = 0;
			virtual VkPipelineLayout get_layout() = 0;
			virtual rhi::PipelineType get_type() = 0;
	};
	
	class IVulkanDevice
	{
		public:
			virtual ~IVulkanDevice() { }

			// Maybe I shouldn't make those variables private
			virtual VkSurfaceKHR get_surface() = 0;
			virtual VkPhysicalDevice get_physical_device() = 0;
			virtual VkDevice get_device() = 0;

			virtual uint32_t get_max_multiview_view_count() = 0;

			virtual IVulkanQueue* get_graphics_queue() = 0;
			virtual IVulkanQueue* get_present_queue() = 0;
			virtual IVulkanQueue* get_compute_queue() = 0;
			virtual IVulkanQueue* get_transfer_queue() = 0;
	};

	class IVulkanSwapChain
	{
		public:
			virtual VkSwapchainKHR get_swap_chain() = 0;
			virtual VkFormat get_format() = 0;
			virtual std::vector<rhi::TextureView>& get_texture_views() = 0;

			// Only for test
			virtual std::vector<VkImageView>& get_image_views() = 0;
			virtual std::vector<VkImage> get_images() = 0;
	};

}