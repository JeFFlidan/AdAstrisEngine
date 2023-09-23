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

void vulkan::VulkanQueue::submit(VulkanCommandManager& cmdManager, bool useSignalSemaphores)
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
		if (useSignalSemaphores)
		{
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &cmdBuffer->_signalSemaphore;
		}
		else
		{
			info.signalSemaphoreCount = 0;
			info.pSignalSemaphores = nullptr;
		}
		info.waitSemaphoreCount = cmdBuffer->_waitSemaphores.size();
		info.pWaitSemaphores = cmdBuffer->_waitSemaphores.data();
		info.pWaitDstStageMask = cmdBuffer->_waitFlags.data();
		submitInfos.push_back(info);
		
		if (_queueType == rhi::QueueType::GRAPHICS)
			_presentWaitSemaphores.push_back(cmdBuffer->_signalSemaphore);
	}

	// TODO need to use the correct fence
	VK_CHECK(vkQueueSubmit(_queue, submitInfos.size(), submitInfos.data(), cmdManager.get_free_fence()));
}

bool vulkan::VulkanQueue::present(VulkanSwapChain* swapChain, uint32_t currentImageIndex)
{
	if (_queueType != rhi::QueueType::GRAPHICS)
	{
		LOG_INFO("VulkanQueue::present(): Can't present non graphics queue")
		return false;
	}
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	VkSwapchainKHR swapchain = swapChain->get_swap_chain();
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = _presentWaitSemaphores.data();
	presentInfo.waitSemaphoreCount = _presentWaitSemaphores.size();
	presentInfo.pImageIndices = &currentImageIndex;
	VkResult result = vkQueuePresentKHR(_queue, &presentInfo);
	_presentWaitSemaphores.clear();
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return false;
	
	return true;
}
