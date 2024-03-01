#pragma once

#include "rhi/resources.h"
#include "engine/vulkan_rhi_module.h"
#include "vulkan_api.h"

namespace ad_astris::vulkan
{
	class VulkanCommandManager;
	class VulkanSwapChain;
	class VulkanDevice;

	class VulkanQueue
	{
		public:
			VulkanQueue(VulkanDevice* device, uint32_t queueFamily, rhi::QueueType queueType, bool isSparseBindingSupported);
		
			void submit(VulkanCommandManager& cmdManager, bool useSignalSemaphores);
			bool present(const std::vector<VulkanSwapChain*>& swapChains);
			void cleanup_present_wait_semaphores() { _presentWaitSemaphores.clear(); }

			VkQueue get_handle() const { return _queue; }
			uint32_t get_family() const { return _family; }
			rhi::QueueType get_type() const { return _queueType; }
			uint32_t get_submission_counter() const { return _submissionCounter; }
		
		private:
			VkQueue _queue{ VK_NULL_HANDLE };
			uint32_t _family;
			rhi::QueueType _queueType;
			bool _isSparseBindingSupported;
			std::vector<VkSemaphore> _presentWaitSemaphores;

			// See in UE5 code, maybe I'll remove it
			uint32_t _submissionCounter{ 0 };
	};
}