#include "memory_utils.h"

#ifdef _WIN32
	#include <malloc.h>
#endif

using namespace ad_astris;

void* MemoryUtils::allocate_aligned_memory(size_t size, size_t alignment)
{
#if defined(_WIN32)
	return _aligned_malloc(size, alignment);
#endif
}

void MemoryUtils::free_aligned_memory(void* ptr)
{
#if defined(_WIN32)
	_aligned_free(ptr);
#endif
}
