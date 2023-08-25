#pragma once

#include "api.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_swap_chain.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

// Maybe temporary solution. I will decide how much threads renderer should have
#define RENDER_THREAD_COUNT 4

namespace ad_astris::vulkan
{
	class VulkanCommandPool;
	class VulkanCommandManager;
	class VulkanQueue;
	
	class VK_RHI_API VulkanCommandBuffer
	{
		friend VulkanCommandPool;
		friend VulkanCommandManager;
		friend VulkanQueue;
		
		public:
			VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, VkPipelineStageFlags waitFlag);
			~VulkanCommandBuffer();

			VkCommandBuffer get_handle() { return _cmdBuffer; }
		
		private:
			VulkanDevice* _device;
			VkCommandBuffer _cmdBuffer;
			VkSemaphore _signalSemaphore;
			VkPipelineStageFlags _stageFlag;
			std::vector<VkSemaphore> _waitSemaphores;
			std::vector<VkPipelineStageFlags> _waitFlags;
			// Need to solve problem with fences
			// VkFence _cmdFence;
	};

	// One VulkanCommandPool per queue per thread per frame
	class VK_RHI_API VulkanCommandPool
	{
		friend VulkanCommandManager;
		friend VulkanQueue;
		
		public:
			VulkanCommandPool(VulkanDevice* device, VulkanQueue* queue);
			~VulkanCommandPool();

			VulkanCommandBuffer* get_cmd_buffer();
			// After calling this method, _usedCmdBuffers is cleared
			// so for the next call you record new command buffers
			void clear_after_submission();
			void flush_submitted_cmd_buffers();
			
		private:
			VulkanDevice* _device{ nullptr };
			VkCommandPool _cmdPool;
			VkPipelineStageFlags _waitFlag;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _freeCmdBuffers;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _usedCmdBuffers;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _submittedCmdBuffers;
	};
	
	void VK_RHI_API clear_after_submission(
		std::vector<std::unique_ptr<VulkanCommandPool>>& freePools,
		std::vector<std::unique_ptr<VulkanCommandPool>>& lockedPools);
	
	class VK_RHI_API VulkanCommandManager
	{
		friend VulkanQueue;
		
		public:
			VulkanCommandManager(VulkanDevice* device, VulkanSwapChain* swapChain);
			void cleanup();
		
			VulkanCommandBuffer* get_command_buffer(rhi::QueueType queueType = rhi::QueueType::GRAPHICS);
			// cmd waits for waitForCmd
			void wait_for_cmd_buffer(VulkanCommandBuffer* cmd, VulkanCommandBuffer* waitForCmd);
			void submit(rhi::QueueType queueType);
		
			/** Reset all command buffers which are dedicated for particular frame.
			 * Should be called only in the beginning of the frame.
			 * @param imageIndex should be received from vkAcquireNextImageKHR
			 */
			void flush_cmd_buffers(uint32_t imageIndex);

			bool acquire_next_image(
				VulkanSwapChain* swapChain,
				uint32_t& nextImageIndex,
				uint32_t currentFrameIndex);

			VkFence get_free_fence();
		
		private:
			VulkanDevice* _device;
			std::vector<VkSemaphore> _acquireSemaphores;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeGraphicsCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedGraphicsCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeTransferCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedTransferCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeComputeCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedComputeCmdPools;
			std::vector<std::vector<VkFence>> _freeFences;
			std::vector<std::vector<VkFence>> _lockedFences;
			uint32_t _imageIndex;
	};
}
