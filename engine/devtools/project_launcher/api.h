#pragma once

#ifdef PROJECT_LAUNCHER_EXPORT
	#define PROJECT_LAUNCHER_API __declspec(dllexport)
#else
	#define PROJECT_LAUNCHER_API __declspec(dllimport)
#endif