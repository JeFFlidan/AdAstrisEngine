#pragma once

#include "memory_utils.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <cstdint>

// Pool allocators were taken from https://github.com/Themaister/Granite/tree/master
namespace ad_astris
{
	template<typename T>
	class PoolAllocator
	{
		public:
			template<typename... ARGS>
			T* allocate(ARGS&&... args)
			{
				if (_freePointers.empty())
				{
					uint32_t objectsCount = 64u << _memoryBlobs.size();
					T* memoryBlob = static_cast<T*>(MemoryUtils::allocate_aligned_memory(objectsCount * sizeof(T),
						std::max<size_t>(64, alignof(T))));
					
					if (!memoryBlob)
						return nullptr;

					_freePointers.reserve(objectsCount);
					for (uint32_t i = 0; i < objectsCount; ++i)
					{
						_freePointers.push_back(&memoryBlob[i]);
					}

					_memoryBlobs.emplace_back(memoryBlob);
				}

				T* objectPtr = _freePointers.back();
				_freePointers.pop_back();
				new(objectPtr) T(std::forward<ARGS>(args)...);
				++_allocatedObjectsCount;
				return objectPtr;
			}

			void free(T* ptr)
			{
				ptr->~T();
				_freePointers.push_back(ptr);
				--_allocatedObjectsCount;
			}

			void cleanup()
			{
				_freePointers.clear();
				_memoryBlobs.clear();
			}

			uint32_t get_pool_size()
			{
				uint32_t objectsCount = 0;
				for (uint32_t i = 0; i != _memoryBlobs.size(); ++i)
				{
					objectsCount += 64u << i;
				}

				return objectsCount * sizeof(T);
			}

			uint32_t get_allocated_objects_count()
			{
				return _allocatedObjectsCount;
			}

		protected:
			std::vector<T*> _freePointers;

			std::vector<std::unique_ptr<T, MemoryUtils::MallocDeleter<T>>> _memoryBlobs;
			uint32_t _allocatedObjectsCount = 0;
	};

	template<typename T>
	class ThreadSafePoolAllocator : private PoolAllocator<T>
	{
		public:
			template<typename... ARGS>
			T* allocate(ARGS&&... args)
			{
				std::lock_guard<std::mutex> blocker{ _threadsLock };
				return PoolAllocator<T>::allocate(std::forward<ARGS>(args)...);
			}

			void free(T* ptr)
			{
				ptr->~T();
				std::lock_guard<std::mutex> blocker{ _threadsLock };
				this->_freePointers.push_back(ptr);
			}

			void cleanup()
			{
				std::lock_guard<std::mutex> blocker{ _threadsLock };
				PoolAllocator<T>::cleanup();
			}

			uint32_t get_pool_size()
			{
				return PoolAllocator<T>::get_pool_size();
			}

			uint32_t get_allocated_objects_count()
			{
				return PoolAllocator<T>::get_allocated_objects_count();
			}

		private:
			std::mutex _threadsLock;
	};
}