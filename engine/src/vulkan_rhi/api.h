#pragma once

#ifdef VK_RHI_EXPORT
	#define VK_RHI_API __declspec(dllexport)
#else
	#define VK_RHI_API __declspec(dllimport)
#endif