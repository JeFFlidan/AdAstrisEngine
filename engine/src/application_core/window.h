#pragma once

#include <string>
#include <windows.h>

namespace ad_astris::acore
{
	struct WindowCreationContext
	{
		std::string windowTitle;
		uint32_t width;
		uint32_t height;
		bool isResizable;
	};
	
	class IWindow
	{
		public:
			virtual ~IWindow() { }
			virtual bool process_messages() = 0;
			virtual HWND get_hWnd() = 0;
	};
}