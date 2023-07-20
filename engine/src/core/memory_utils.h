#pragma once

#include <cstdint>

namespace ad_astris
{
	class MemoryUtils
	{
		public:
			static void* allocate_aligned_memory(size_t size, size_t alignment);
			static void free_aligned_memory(void* ptr);

			template<typename T>
			struct MallocDeleter
			{
				void operator()(T* ptr)
				{
					MemoryUtils::free_aligned_memory(ptr);
				}
			};
	};
}