#pragma once

#include "api.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_swap_chain.h"
#include <vulkan/vulkan.h>
#include <map>
#include <vector>

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
			VkFence _cmdFence;
	};

	// One VulkanCommandPool per queue per thread per frame
	class VK_RHI_API VulkanCommandPool
	{
		friend VulkanCommandManager;
		friend VulkanQueue;
		
		public:
			VulkanCommandPool(VulkanDevice* device, IVulkanQueue* queue);
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
			std::vector<VulkanCommandBuffer*> _freeCmdBuffers;
			std::vector<VulkanCommandBuffer*> _usedCmdBuffers;
			std::vector<VulkanCommandBuffer*> _submittedCmdBuffers;
	};
	
	void VK_RHI_API clear_after_submission(
		std::vector<VulkanCommandPool*>& freePools,
		std::vector<VulkanCommandPool*>& lockedPools);
	
	class VK_RHI_API VulkanCommandManager
	{
		friend VulkanQueue;
		
		public:
			VulkanCommandManager(VulkanDevice* device, VulkanSwapChain* swapChain);
			~VulkanCommandManager();
		
			VulkanCommandBuffer* get_command_buffer(rhi::QueueType queueType = rhi::QueueType::GRAPHICS);
			// cmd waits for waitForCmd
			void wait_for_cmd_buffer(VulkanCommandBuffer* cmd, VulkanCommandBuffer* waitForCmd);
			void submit(rhi::QueueType queueType);
		
			/** Reset all command buffers which are dedicated for particular frame.
			 * Should be called only in the beginning of the frame.
			 * @param imageIndex should be received from vkAcquireNextImageKHR
			 */
			void flush_cmd_buffers(uint32_t imageIndex);
		private:
			VulkanDevice* _device;
			std::vector<VkSemaphore> _acquireSemaphores;
			VkFence fence;		// Need to be remade
			std::vector<std::vector<VulkanCommandPool*>> _freeGraphicsCmdPools;
			std::vector<std::vector<VulkanCommandPool*>> _lockedGraphicsCmdPools;
			std::vector<std::vector<VulkanCommandPool*>> _freeTransferCmdPools;
			std::vector<std::vector<VulkanCommandPool*>> _lockedTransferCmdPools;
			std::vector<std::vector<VulkanCommandPool*>> _freeComputeCmdPools;
			std::vector<std::vector<VulkanCommandPool*>> _lockedComputeCmdPools;
			uint32_t _imageIndex;
	};
}
