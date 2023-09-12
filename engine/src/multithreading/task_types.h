#pragma once

#include "profiler/logger.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <functional>
#include <unordered_set>

namespace ad_astris::tasks
{
	struct TaskExecutionInfo
	{
		uint32_t globalTaskIndex;
		uint32_t taskSubgroupID;
		uint32_t taskIndexRelativeToSubgroup;
		bool isFirstTaskInSubgroup;
		bool isLastTaskInSubgroup;
	};

	using TaskHandler = std::function<void(TaskExecutionInfo)>;
	
	class TaskGroup
	{
		public:
			void increase_task_count(uint32_t taskCount)
			{
				_pendingTaskCount.fetch_add(taskCount);
				_isDependenciesResolved = false;
			}

			void decrease_task_count(uint32_t taskCount)
			{
				_pendingTaskCount.fetch_sub(taskCount);
				if (_pendingTaskCount.load() == 0)
				{
					_waitCondition.notify_all();
					_isDependenciesResolved = true;
				}
			}

			uint32_t get_pending_task_count()
			{
				return _pendingTaskCount.load();
			}

			void wait_for_other_groups()
			{
				if (!_isDependenciesResolved)
				{
					for (auto& dependency : _dependencies)
					{
						std::unique_lock<std::mutex> lock(dependency->_mutex);
						dependency->_waitCondition.wait(lock);
					}
				}
			}

			void add_dependency(TaskGroup* taskGroup)
			{
				if (taskGroup)
					_dependencies.insert(taskGroup);
			}

			void remove_dependency(TaskGroup* taskGroup)
			{
				auto it = _dependencies.find(taskGroup);
				if (it == _dependencies.end())
					LOG_FATAL("TaskGroup::remove_dependency(): Task group doesn't have dependency that you want to remove")
				_dependencies.erase(taskGroup);
			}
		
		private:
			std::atomic<uint32_t> _pendingTaskCount{ 0 };
			std::condition_variable _waitCondition;
			std::mutex _mutex;
			std::unordered_set<TaskGroup*> _dependencies;		// Task group waits for these task groups
			bool _isDependenciesResolved{ true };
	};
	
	struct Task
	{
		TaskHandler taskHandler;
		TaskGroup* taskGroup;
		uint32_t taskSubgroupID;
		uint32_t taskSubgroupBeginning;
		uint32_t taskSubgroupEnd;
	};
	
	class TaskQueue
	{
		public:
			void push_back(const Task& task)
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				_tasks.push_back(task);
			}

			void pop_front(Task& task)
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				task = _tasks.front();
				_tasks.pop_front();
			}

			bool empty()
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				return _tasks.empty();
			}
		
		private:
			std::deque<Task> _tasks;
			std::mutex _mutex;
	};

	class TaskQueueGroup
	{
		public:
			TaskQueueGroup(uint32_t threadCount) : _threadCount(threadCount)
			{
				_taskQueues.reserve(threadCount);
				for (auto i = 0; i != threadCount; ++i)
					_taskQueues.emplace_back(new TaskQueue());
			}

			TaskQueue& get_queue(uint32_t index)
			{
				return *_taskQueues[index % _threadCount];
			}
		
			TaskQueue& get_next_queue()
			{
				return *_taskQueues[_nextQueue.fetch_add(1) % _threadCount];
			}

			uint32_t get_next_queue_index()
			{
				return _nextQueue.fetch_add(1) % _threadCount;
			}

		private:
			std::vector<std::unique_ptr<TaskQueue>> _taskQueues;
			std::atomic<uint64_t> _nextQueue{ 0 };
			uint32_t _threadCount;
	};
}