#include "vulkan_queue.h"
#include "vulkan_common.h"
#include "vulkan_command_manager.h"
#include "vulkan_swap_chain.h"

using namespace ad_astris;

vulkan::VulkanQueue::VulkanQueue(VulkanDevice* device, uint32_t queueFamily, rhi::QueueType queueType, bool isSparseBindingSupported)
	: _family(queueFamily), _queueType(queueType), _isSparseBindingSupported(isSparseBindingSupported)
{
	VkDeviceQueueInfo2 queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	queueInfo.queueIndex = 0;
	queueInfo.queueFamilyIndex = queueFamily;
	vkGetDeviceQueue2(device->get_device(), &queueInfo, &_queue);
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

	std::vector<VkSubmitInfo2> submitInfos;
	std::vector<std::vector<VkCommandBufferSubmitInfo>> allCmdSubmitInfos;
	std::vector<std::vector<VkSemaphoreSubmitInfo>> allSemaphoreSubmitInfos;
	for (auto& pool : *cmdPools)
	{
		VulkanCommandBuffer* cmdBuffer = pool->_usedCmdBuffers[0].get();
		vkEndCommandBuffer(cmdBuffer->get_handle());

		std::vector<VkCommandBufferSubmitInfo>& cmdSubmitInfos = allCmdSubmitInfos.emplace_back();
		VkCommandBufferSubmitInfo& cmdSubmitInfo = cmdSubmitInfos.emplace_back();
		cmdSubmitInfo.commandBuffer = cmdBuffer->get_handle();
		cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdSubmitInfo.deviceMask = 0;

		std::vector<VkSemaphoreSubmitInfo>& waitSemaphoresSubmitInfos = allSemaphoreSubmitInfos.emplace_back();
		for (size_t i = 0; i != cmdBuffer->_waitSemaphores.size(); ++i)
		{
			VkSemaphoreSubmitInfo& info = waitSemaphoresSubmitInfos.emplace_back();
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			info.semaphore = cmdBuffer->_waitSemaphores[i];
			info.stageMask = cmdBuffer->_waitFlags[i];
			info.deviceIndex = 0;
		}
		
		VkSubmitInfo2& submitInfo = submitInfos.emplace_back();
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = cmdSubmitInfos.size();
		submitInfo.pCommandBufferInfos = cmdSubmitInfos.data();
		submitInfo.waitSemaphoreInfoCount = waitSemaphoresSubmitInfos.size();
		submitInfo.pWaitSemaphoreInfos = waitSemaphoresSubmitInfos.data();

		if (useSignalSemaphores)
		{
			std::vector<VkSemaphoreSubmitInfo>& signalSemaphoresSubmitInfos = allSemaphoreSubmitInfos.emplace_back();
			VkSemaphoreSubmitInfo& semaphoreSubmitInfo = signalSemaphoresSubmitInfos.emplace_back();
			semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreSubmitInfo.semaphore = cmdBuffer->_signalSemaphore;
			semaphoreSubmitInfo.stageMask = cmdBuffer->_stageFlag;
			semaphoreSubmitInfo.deviceIndex = 0;
			submitInfo.signalSemaphoreInfoCount = signalSemaphoresSubmitInfos.size();
			submitInfo.pSignalSemaphoreInfos = signalSemaphoresSubmitInfos.data();
		}
		
		if (_queueType == rhi::QueueType::GRAPHICS)
			_presentWaitSemaphores.push_back(cmdBuffer->_signalSemaphore);
	}

	// TODO need to use the correct fence
	VK_CHECK(vkQueueSubmit2(_queue, submitInfos.size(), submitInfos.data(), cmdManager.get_free_fence()));
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
	VkSwapchainKHR swapchain = swapChain->get_handle();
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = _presentWaitSemaphores.data();
	presentInfo.waitSemaphoreCount = _presentWaitSemaphores.size();
	presentInfo.pImageIndices = &currentImageIndex;
	VkResult result = vkQueuePresentKHR(_queue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return false;
	
	return true;
}
