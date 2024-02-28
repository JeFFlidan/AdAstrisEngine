#pragma once

#ifndef VK_NO_PROTOTYPES
	#define VK_NO_PROTOTYPES
#endif
#ifdef _WIN32
	#define NOMINMAX
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif
#include <volk/volk.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vma/vk_mem_alloc.h>
