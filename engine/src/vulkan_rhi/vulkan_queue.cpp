#include "vulkan_queue.h"
#include "vulkan_common.h"

using namespace ad_astris;

vulkan::VulkanQueue::VulkanQueue(QueueData queueData)
{
	_queue = queueData.queue;
	_family = queueData.queueFamily;
	_queueType = queueData.queueType;
	_submissionCounter = 0;
}

void vulkan::VulkanQueue::submit(VulkanCommandManager& cmdManager)
{
	std::vector<std::unique_ptr<VulkanCommandPool>>* cmdPools;
	switch (_queueType)
	{
		case rhi::QueueType::GRAPHICS:
		{
			cmdPools = &cmdManager._lockedGraphicsCmdPools[cmdManager._imageIndex];
			break;
		}
		case rhi::QueueType::COMPUTE:
		{
			cmdPools = &cmdManager._lockedComputeCmdPools[cmdManager._imageIndex];
			break;
		}
		case rhi::QueueType::TRANSFER:
		{
			cmdPools = &cmdManager._lockedTransferCmdPools[cmdManager._imageIndex];
			break;
		}
	}

	std::vector<VkSubmitInfo> submitInfos;
	for (auto& pool : *cmdPools)
	{
		VulkanCommandBuffer* cmdBuffer = pool->_usedCmdBuffers[0].get();
		vkEndCommandBuffer(cmdBuffer->get_handle());
		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &cmdBuffer->_cmdBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &cmdBuffer->_signalSemaphore;
		info.waitSemaphoreCount = cmdBuffer->_waitSemaphores.size();
		info.pWaitSemaphores = cmdBuffer->_waitSemaphores.data();
		info.pWaitDstStageMask = cmdBuffer->_waitFlags.data();
		submitInfos.push_back(info);
	}

	// TODO need to use the correct fence
	VK_CHECK(vkQueueSubmit(_queue, submitInfos.size(), submitInfos.data(), cmdManager.get_free_fence()));
}

void vulkan::VulkanQueue::present()
{
	// TODO
}
