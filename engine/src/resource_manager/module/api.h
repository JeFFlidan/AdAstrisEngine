#pragma once

#ifdef RESOURCE_MANAGER_API
	#define RESOURCE_MANAGER_API __declspec(dllexport)
#else
	#define RESOURCE_MANAGER_API __declspec(dllimport)
#endif