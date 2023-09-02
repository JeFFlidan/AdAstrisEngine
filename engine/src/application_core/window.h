#pragma once

#include <string>

#ifdef _WIN32
	struct HWND__;
	typedef HWND__* HWND;
#endif

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