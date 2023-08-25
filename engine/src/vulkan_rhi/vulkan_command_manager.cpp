#include "vulkan_command_manager.h"
#include "vulkan_queue.h"
#include "vulkan_common.h"
#include <utility>

using namespace ad_astris;

vulkan::VulkanCommandBuffer::VulkanCommandBuffer(
	VulkanDevice* device,
	VkCommandPool pool,
	VkPipelineStageFlags waitFlag) : _device(device), _stageFlag(waitFlag)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = pool;
	VK_CHECK(vkAllocateCommandBuffers(_device->get_device(), &allocInfo, &_cmdBuffer));

	create_semaphore(_device->get_device(), &_signalSemaphore);
}

vulkan::VulkanCommandBuffer::~VulkanCommandBuffer()
{
	vkDestroySemaphore(_device->get_device(), _signalSemaphore, nullptr);
}

vulkan::VulkanCommandPool::VulkanCommandPool(VulkanDevice* device, VulkanQueue* queue) : _device(device)
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = 0;
	info.queueFamilyIndex = queue->get_family();
	VK_CHECK(vkCreateCommandPool(_device->get_device(), &info, nullptr, &_cmdPool));

	// Need to think about flags
	switch (queue->get_queue_type())
	{
		case rhi::QueueType::GRAPHICS:
		{
			_waitFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case rhi::QueueType::COMPUTE:
		{
			_waitFlag = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			break;
		}
		case rhi::QueueType::TRANSFER:
		{
			_waitFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
	}
}

vulkan::VulkanCommandPool::~VulkanCommandPool()
{
	vkDestroyCommandPool(_device->get_device(), _cmdPool, nullptr);
}

vulkan::VulkanCommandBuffer* vulkan::VulkanCommandPool::get_cmd_buffer()
{
	VulkanCommandBuffer* cmdBuffer;
	if (!_freeCmdBuffers.empty())
	{
		cmdBuffer = _freeCmdBuffers.back().get();
		_usedCmdBuffers.emplace_back(std::move(_freeCmdBuffers.back()));
		_freeCmdBuffers.pop_back();
	}
	else
	{
		_usedCmdBuffers.emplace_back(new VulkanCommandBuffer(_device, _cmdPool, _waitFlag));
		cmdBuffer = _usedCmdBuffers.back().get();
	}
	return cmdBuffer;
}

void vulkan::VulkanCommandPool::clear_after_submission()
{
	for (auto& cmdBuffer : _usedCmdBuffers)
	{
		cmdBuffer->_waitSemaphores.clear();
		cmdBuffer->_waitFlags.clear();
	}
	_submittedCmdBuffers = std::move(_usedCmdBuffers);			// TODO FIX BUG
}

void vulkan::VulkanCommandPool::flush_submitted_cmd_buffers()
{
	vkResetCommandPool(_device->get_device(), _cmdPool, 0);
	_freeCmdBuffers = std::move(_submittedCmdBuffers);
}

vulkan::VulkanCommandManager::VulkanCommandManager(VulkanDevice* device, VulkanSwapChain* swapChain) : _device(device)
{
	VulkanQueue* graphicsQueue = _device->get_graphics_queue();
	VulkanQueue* computeQueue = _device->get_compute_queue();
	VulkanQueue* transferQueue = _device->get_transfer_queue();

	size_t buffersCount = swapChain->get_texture_views().size();
	size_t poolsPerQueue = RENDER_THREAD_COUNT * buffersCount;
	
	for (int i = 0; i != 3; ++i)
	{
		std::vector<std::unique_ptr<VulkanCommandPool>> tempPools;
		for (size_t j = 1; j <= poolsPerQueue; ++j)
		{
			switch (i)
			{
				case 0:
				{
					tempPools.emplace_back(new VulkanCommandPool(_device, graphicsQueue));
					if (tempPools.size() == poolsPerQueue / buffersCount)
					{
						_freeGraphicsCmdPools.push_back(std::move(tempPools));
						//tempPools.clear();
					}
					break;
				}
				case 1:
				{
					tempPools.emplace_back(new VulkanCommandPool(_device, computeQueue));
					if (tempPools.size() == poolsPerQueue / buffersCount)
					{
						_freeComputeCmdPools.push_back(std::move(tempPools));
						//tempPools.clear();
					}
					break;
				}
				case 2:
				{
					tempPools.emplace_back(new VulkanCommandPool(_device, transferQueue));
					if (tempPools.size() == poolsPerQueue / buffersCount)
					{
						_freeTransferCmdPools.push_back(std::move(tempPools));
						//tempPools.clear();
					}
					break;
				}
			}
		}
	}
	LOG_INFO("First frame graphics: {}", _freeGraphicsCmdPools[0].size())
	LOG_INFO("Second frame graphics: {}", _freeGraphicsCmdPools[1].size())

	LOG_INFO("First frame transfer: {}", _freeTransferCmdPools[0].size())
	LOG_INFO("Second frame transfer: {}", _freeTransferCmdPools[1].size())

	LOG_INFO("First frame compute: {}", _freeComputeCmdPools[0].size())
	LOG_INFO("Second frame compute: {}", _freeComputeCmdPools[1].size())

	for (size_t i = 0; i != buffersCount; ++i)
	{
		VkSemaphore semaphore;
		create_semaphore(_device->get_device(), &semaphore);
		_acquireSemaphores.push_back(semaphore);
	}
}

void vulkan::VulkanCommandManager::cleanup()
{
	for (auto& semaphore : _acquireSemaphores)
	{
		vkDestroySemaphore(_device->get_device(), semaphore, nullptr);
	}

	for (auto& fence : _freeFences[_imageIndex])
		vkDestroyFence(_device->get_device(), fence, nullptr);
}

vulkan::VulkanCommandBuffer* vulkan::VulkanCommandManager::get_command_buffer(rhi::QueueType queueType)
{
	VulkanCommandBuffer* cmdBuffer;
	switch (queueType)
	{
		case rhi::QueueType::GRAPHICS:
		{
			if (!_freeGraphicsCmdPools[_imageIndex].empty())
			{
				LOG_FATAL("VulkanCommandManage::get_command_buffer(): Can't dedicate more than 4 threads")
			}

			cmdBuffer = _freeGraphicsCmdPools[_imageIndex].back()->get_cmd_buffer();
			_lockedGraphicsCmdPools[_imageIndex].push_back(std::move(_freeGraphicsCmdPools[_imageIndex].back()));
			_freeGraphicsCmdPools.pop_back();
			break;
		}
		case rhi::QueueType::COMPUTE:
		{
			if (!_freeComputeCmdPools[_imageIndex].empty())
			{
				LOG_FATAL("VulkanCommandManage::get_command_buffer(): Can't dedicate more than 4 threads")
			}

			cmdBuffer = _freeComputeCmdPools[_imageIndex].back()->get_cmd_buffer();
			_lockedComputeCmdPools[_imageIndex].push_back(std::move(_freeComputeCmdPools[_imageIndex].back()));
			_freeComputeCmdPools.pop_back();
			break;
		}
		case rhi::QueueType::TRANSFER:
		{
			if (!_freeTransferCmdPools[_imageIndex].empty())
			{
				LOG_FATAL("VulkanCommandManage::get_command_buffer(): Can't dedicate more than 4 threads")
			}

			cmdBuffer = _freeTransferCmdPools[_imageIndex].back()->get_cmd_buffer();
			_lockedTransferCmdPools[_imageIndex].push_back(std::move(_freeTransferCmdPools[_imageIndex].back()));
			_freeTransferCmdPools.pop_back();
			break;
		}
	}

	// TODO VkPipelineStageFlags. Maybe I have to remove it because the same flag is set up in get_cmd_buffer method of VulkanCommandPool
	cmdBuffer->_waitSemaphores.push_back(_acquireSemaphores[_imageIndex]);
	VkPipelineStageFlags flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	cmdBuffer->_waitFlags.push_back(flag);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pInheritanceInfo = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer->get_handle(), &beginInfo);
	return cmdBuffer;
}

void vulkan::VulkanCommandManager::wait_for_cmd_buffer(VulkanCommandBuffer* cmd, VulkanCommandBuffer* waitForCmd)
{
	cmd->_waitSemaphores.push_back(waitForCmd->_signalSemaphore);
	cmd->_waitFlags.push_back(waitForCmd->_stageFlag);
}

void vulkan::VulkanCommandManager::submit(rhi::QueueType queueType)
{
	switch (queueType)
	{
		case rhi::QueueType::GRAPHICS:
		{
			_device->get_graphics_queue()->submit(*this);
			auto& lockedPools = _lockedGraphicsCmdPools[_imageIndex];
			auto& freePools = _freeGraphicsCmdPools[_imageIndex];
			clear_after_submission(freePools, lockedPools);
			break;
		}
		case rhi::QueueType::COMPUTE:
		{
			_device->get_compute_queue()->submit(*this);
			auto& lockedPools = _lockedComputeCmdPools[_imageIndex];
			auto& freePools = _freeComputeCmdPools[_imageIndex];
			clear_after_submission(freePools, lockedPools);
			break;
		}
		case rhi::QueueType::TRANSFER:
		{
			_device->get_transfer_queue()->submit(*this);
			auto& lockedPools = _lockedTransferCmdPools[_imageIndex];
			auto& freePools = _freeTransferCmdPools[_imageIndex];
			clear_after_submission(freePools, lockedPools);
			break;
		}
	}
}

void vulkan::VulkanCommandManager::flush_cmd_buffers(uint32_t imageIndex)
{
	_imageIndex = imageIndex;
	auto& graphicsPools = _freeGraphicsCmdPools[_imageIndex];
	auto& transferPools = _freeTransferCmdPools[_imageIndex];
	auto& computePools = _freeComputeCmdPools[_imageIndex];

	for (int i = 0; i != RENDER_THREAD_COUNT; ++i)
	{
		graphicsPools[i]->flush_submitted_cmd_buffers();
		transferPools[i]->flush_submitted_cmd_buffers();
		computePools[i]->flush_submitted_cmd_buffers();
	}
}

bool vulkan::VulkanCommandManager::acquire_next_image(VulkanSwapChain* swapChain, uint32_t& nextImageIndex, uint32_t currentFrameIndex)
{
	uint32_t lockedFencesCount = _lockedFences[currentFrameIndex].size();
	VK_CHECK(vkWaitForFences(_device->get_device(), lockedFencesCount, _lockedFences[currentFrameIndex].data(), true, 1000000000));
	VK_CHECK(vkResetFences(_device->get_device(), lockedFencesCount, _lockedFences[currentFrameIndex].data()));
	
	if (vkAcquireNextImageKHR(_device->get_device(), swapChain->get_swap_chain(), 1000000000, _acquireSemaphores[currentFrameIndex], VK_NULL_HANDLE, &nextImageIndex) == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return false;
	}

	_imageIndex = nextImageIndex;

	return true;
}

VkFence vulkan::VulkanCommandManager::get_free_fence()
{
	std::vector<VkFence>& freeFences = _freeFences[_imageIndex];
	if (freeFences.empty())
		freeFences.push_back(create_fence(_device->get_device()));
	VkFence fence = _freeFences[_imageIndex].back();
	_freeFences[_imageIndex].pop_back();
	return fence;
}

void vulkan::clear_after_submission(
	std::vector<std::unique_ptr<VulkanCommandPool>>& freePools,
	std::vector<std::unique_ptr<VulkanCommandPool>>& lockedPools)
{
	for (auto& pool : lockedPools)
	{
		pool->clear_after_submission();
		freePools.push_back(std::move(pool));
	}
	lockedPools.clear();
}
