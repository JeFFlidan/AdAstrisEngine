#pragma once

#ifdef RENDER_CORE_EXPORT
	#define RENDER_CORE_API __declspec(dllexport)
#else
	#define RENDER_CORE_API __declspec(dllimport)
#endif