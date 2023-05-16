#pragma once

#ifdef ECS_EXPORT
	#define ECS_API __declspec(dllexport)
#else
	#define ECS_API __declspec(dllimport)
#endif