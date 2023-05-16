#pragma once

#include "vulkan_command_manager.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	struct QueueData
	{
		VkQueue queue;
		uint32_t queueFamily;
		rhi::QueueType queueType;
	};

	class VulkanQueue
	{
		public:
			VulkanQueue(QueueData queueData);
		
			void submit(VulkanCommandManager& cmdManager);
			void present();

			VkQueue get_queue() { return _queue; }
			uint32_t get_family() { return _family; }
			rhi::QueueType get_queue_type() { return _queueType; }
			uint32_t get_submission_counter() { return _submissionCounter; }
		private:
			VkQueue _queue;
			uint32_t _family;
			rhi::QueueType _queueType;

			// See in UE5 code, maybe I'll remove it
			uint32_t _submissionCounter;
	};
}