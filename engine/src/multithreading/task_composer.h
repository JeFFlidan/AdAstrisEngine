#pragma once

#include "task_types.h"
#include "core/pool_allocator.h"

#include <condition_variable>
#include <thread>

namespace ad_astris::tasks
{
	class TaskComposer
	{
		public:
			explicit TaskComposer(uint32_t threadCount = ~0u);
			~TaskComposer();
			void execute(TaskGroup& taskGroup, const TaskHandler& taskHandler);
			void dispatch(TaskGroup& taskGroup, uint32_t taskCount, uint32_t groupSize, const TaskHandler& taskHandler);
			bool is_busy(TaskGroup& taskGroup);
			void wait(TaskGroup& taskGroup);

			uint32_t get_thread_count()
			{
				return _threadCount;
			}
		
		private:
			std::unique_ptr<TaskQueueGroup> _taskQueueGroup{ nullptr };
			std::vector<std::thread> _threads;
			std::condition_variable _wakeCondition;
			std::mutex _mutex;
			uint32_t _threadCount;
			std::atomic_bool _isAlive{ true };

			void execute_tasks(uint32_t beginningQueueIndex);
			uint32_t calculate_group_count(uint32_t taskCount, uint32_t groupSize);
	};
}