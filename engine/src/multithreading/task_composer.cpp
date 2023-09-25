#include "task_composer.h"
#include "profiler/logger.h"
#include "core/timer.h"
#include <cassert>
#include <exception>
#include <stdio.h>

using namespace ad_astris;
using namespace tasks;

std::vector<TaskHandler> taskHandlers;

TaskComposer::TaskComposer(uint32_t maxThreadCount)
{
	maxThreadCount = std::max(1u, maxThreadCount);

	uint32_t coreCount = std::thread::hardware_concurrency();
	_threadCount = std::min(maxThreadCount, std::max(1u, coreCount - 1));
	LOG_INFO("Thread count: {}", _threadCount)
	_threads.reserve(_threadCount);
	_taskQueueGroup = std::make_unique<TaskQueueGroup>(_threadCount);

	for (auto threadID = 0; threadID != _threadCount; ++threadID)
	{
		_threads.emplace_back([this, threadID]
		{
			while (_isAlive.load())
			{
				execute_tasks(threadID);

				std::unique_lock<std::mutex> lock(_mutex);
				_wakeCondition.wait(lock);
			}
		});
	}

	_taskGroupPool.allocate_new_pool(128);
}

TaskComposer::~TaskComposer()
{
	_isAlive.store(false);
	bool executeWakeLoop = true;

	std::thread waker([&]()
	{
		while (executeWakeLoop)
			_wakeCondition.notify_all();
	});

	for (auto& thread : _threads)
	{
		thread.join();
	}

	executeWakeLoop = false;
	waker.join();
}

void TaskComposer::execute(TaskGroup& taskGroup, const TaskHandler& taskHandler)
{
	taskGroup.increase_task_count(1);

	Task task;
	task.taskHandler = taskHandler;
	task.taskGroup = &taskGroup;
	task.taskSubgroupBeginning = 0;
	task.taskSubgroupEnd = 1;
	task.taskSubgroupID = 0;

	_taskQueueGroup->get_next_queue().push_back(task);
	_wakeCondition.notify_one();
}

void TaskComposer::dispatch(TaskGroup& taskGroup, uint32_t taskCount, uint32_t groupSize, const TaskHandler& taskHandler)
{
	if (taskCount == 0 || groupSize == 0)
		return;

	uint32_t groupCount = calculate_group_count(taskCount, groupSize);
	taskGroup.increase_task_count(groupCount);

	//Task* task = _taskPool.allocate();
	Task task;
	task.taskHandler = taskHandler;
	task.taskGroup = &taskGroup;
	taskHandlers.push_back(taskHandler);
	
	for (auto groupID = 0; groupID != groupCount; ++groupID)
	{
		task.taskSubgroupBeginning = groupID * groupSize;
		task.taskSubgroupEnd = std::min(task.taskSubgroupBeginning + groupSize, taskCount);
		task.taskSubgroupID = groupID;
		_taskQueueGroup->get_next_queue().push_back(task);
	}

	_wakeCondition.notify_all();
}

bool TaskComposer::is_busy(TaskGroup& taskGroup)
{
	return taskGroup.get_pending_task_count() > 0;
}

void TaskComposer::wait(TaskGroup& taskGroup)
{
	if (is_busy(taskGroup))
	{
		_wakeCondition.notify_all();
		execute_tasks(_taskQueueGroup->get_next_queue_index());

		while (is_busy(taskGroup))
		{
			std::this_thread::yield();
		}
	}
}

void TaskComposer::execute_tasks(uint32_t beginningQueueIndex)
{
	TaskExecutionInfo executionInfo;
	Task task;
	for (auto i = 0; i != _threadCount; ++i)
	{
		TaskQueue& taskQueue = _taskQueueGroup->get_queue(beginningQueueIndex);
		while (taskQueue.pop_front(task))
		{
			task.taskGroup->wait_for_other_groups();
			//TaskExecutionInfo* executionInfo = _taskExecutionInfoPool.allocate();
			executionInfo.taskSubgroupID = task.taskSubgroupID;

			for (auto taskIndex = task.taskSubgroupBeginning; taskIndex != task.taskSubgroupEnd; ++taskIndex)
			{
				executionInfo.globalTaskIndex = taskIndex;
				executionInfo.taskIndexRelativeToSubgroup = taskIndex - task.taskSubgroupBeginning;
				executionInfo.isFirstTaskInSubgroup = taskIndex == task.taskSubgroupBeginning;
				executionInfo.isLastTaskInSubgroup = taskIndex == task.taskSubgroupEnd - 1;
				try
				{
					task.taskHandler(executionInfo);
				}
				catch (std::bad_function_call)
				{
					LOG_FATAL("TaskComposer::execute_tasks(): Bad function call. Task index: {}", executionInfo.globalTaskIndex)
				}
			}
			
			task.taskGroup->decrease_task_count(1);
			//_taskPool.free(task);
			//_taskExecutionInfoPool.free(executionInfo);
		}

		++beginningQueueIndex;
	}
}

uint32_t TaskComposer::calculate_group_count(uint32_t taskCount, uint32_t groupSize)
{
	return (taskCount + groupSize - 1) / groupSize;
}
