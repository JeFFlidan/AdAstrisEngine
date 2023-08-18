#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <functional>

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
	
	struct TaskGroup
	{
		std::atomic<uint32_t> pendingTaskCount{ 0 };
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

			Task& pop_front()
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				Task& task = _tasks.front();
				_tasks.pop_front();
				return task;
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