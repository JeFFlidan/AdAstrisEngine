#include "task_composer.h"
#include "profiler/logger.h"
#include "core/timer.h"
#include <cassert>
#include <exception>
#include <stdio.h>

#ifdef _WIN32
	#include <windows.h>
#endif

using namespace ad_astris;
using namespace tasks;

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
	taskGroup.pendingTaskCount.fetch_add(1);

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
	taskGroup.pendingTaskCount.fetch_add(groupCount);

	//Task* task = _taskPool.allocate();
	Task task;
	task.taskHandler = taskHandler;
	task.taskGroup = &taskGroup;
	
	for (auto groupID = 0; groupID != groupCount; ++groupID)
	{
		task.taskSubgroupBeginning = groupID * groupSize;
		task.taskSubgroupEnd = std::min(task.taskSubgroupBeginning + groupSize, taskCount);
		task.taskSubgroupID = groupID;
		_taskQueueGroup->get_queue(groupID).push_back(task);
	}

	_wakeCondition.notify_all();
}

bool TaskComposer::is_busy(TaskGroup& taskGroup)
{
	return taskGroup.pendingTaskCount.load() > 0;
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
	for (auto i = 0; i != _threadCount; ++i)
	{
		TaskQueue& taskQueue = _taskQueueGroup->get_queue(beginningQueueIndex);
		while (!taskQueue.empty())
		{
			Task& task = taskQueue.pop_front();
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
			
			task.taskGroup->pendingTaskCount.fetch_sub(1);
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
