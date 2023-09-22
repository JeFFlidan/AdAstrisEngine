#pragma once

#ifdef EDITOR_EXPORT
	#define EDITOR_API __declspec(dllexport)
#else
	#define EDITOR_API __declspec(dllimport)
#endif