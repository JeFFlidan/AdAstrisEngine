#pragma once

#include "memory_utils.h"

#include <profiler/logger.h>

#include <vector>
#include <algorithm>
#include <cstdint>

namespace ad_astris
{
	template<typename T, int64_t count>
	class StackAllocator
	{
		public:
			~StackAllocator()
			{
				if (_stackBeginning)
					LOG_ERROR("StackAllocator::~StackAllocator(): Stack allocator was not cleaned up. Memory leak!!!")
			}

			template<typename... ARGS>
			T* allocate(ARGS&&... args)
			{
				if (count <= 0)
				{
					LOG_FATAL("StackAllocator::allocate(): Can't create stack allocator if objects count <= 0")
					return nullptr;
				}

				if (!_stackBeginning)
				{
					_stackBeginning = static_cast<T*>(MemoryUtils::allocate_aligned_memory(count * sizeof(T), std::max<size_t>(64, alignof(T))));
				}

				if (_offset >= count)
				{
					LOG_ERROR("StackAllocator::allocate(): Can't allocate any more because stack is full");
					return nullptr;
				}

				T* objectPtr = _stackBeginning + _offset++;
				new(objectPtr) T(std::forward<ARGS>(args)...);

				return objectPtr;
			}

			void cleanup()
			{
				delete[] _stackBeginning;
				_stackBeginning = nullptr;
			}

			void reset()
			{
				_offset = 0;
			}

			uint32_t get_allocated_objects_count()
			{
				return _offset;
			}

			uint32_t get_stack_size()
			{
				return count * sizeof(T);
			}

			uint32_t get_max_objects_count()
			{
				return count;
			}

		protected:
			T* _stackBeginning{ nullptr };
			uint32_t _offset = 0;
	};

}