#pragma once

#include "memory_utils.h"
#include <cstdint>
#include <cassert>

namespace ad_astris
{
	class LinearAllocator
	{
		public:
			~LinearAllocator()
			{
				MemoryUtils::free_aligned_memory(buffer);
			}
			constexpr size_t get_capacity() const
			{
				return capacity;
			}
			void reserve(size_t newCapacity, size_t align)
			{
				alignment = align;
				newCapacity = Align(newCapacity, alignment);
				capacity = newCapacity;

				MemoryUtils::free_aligned_memory(buffer);
				buffer = (uint8_t*)MemoryUtils::allocate_aligned_memory(capacity, alignment);
			}
			constexpr uint8_t* allocate(size_t size)
			{
				size = Align(size, alignment);
				if (offset + size <= capacity)
				{
					uint8_t* ret = &buffer[offset];
					offset += size;
					return ret;
				}
				return nullptr;
			}
			constexpr void free(size_t size)
			{
				size = Align(size, alignment);
				assert(offset >= size);
				offset -= size;
			}
			constexpr void reset()
			{
				offset = 0;
			}
			constexpr uint8_t* top()
			{
				return &buffer[offset];
			}

		private:
			uint8_t* buffer = nullptr;
			size_t capacity = 0;
			size_t offset = 0;
			size_t alignment = 1;

			constexpr size_t Align(size_t value, size_t alignment)
			{
				return ((value + alignment - 1) / alignment) * alignment;
			}
	};
}