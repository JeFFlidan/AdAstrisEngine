#pragma once

#include "window_events.h"
#include "events/event_manager.h"
#include "window.h"
#include <windows.h>

namespace ad_astris::acore::impl
{
	class WinApiWindow : public IWindow
	{
		public:
			WinApiWindow() = default;
			WinApiWindow(const char* windowTitle, events::EventManager* eventManager);
			~WinApiWindow();
			WinApiWindow(const WinApiWindow&) = delete;
			WinApiWindow& operator=(const WinApiWindow&) = delete;

			virtual bool process_messages() override;

			void parse_keys(impl::KeyEvent& keyEvent, WPARAM wParam);

			events::EventManager* get_event_manager()
			{
				return _eventManager;
			}

		private:
			HINSTANCE _hInstance;
			HWND _hWnd;
			RECT _windowRect;
			RECT _clientRect;
			const char* _className;
			int32_t _minWidth{ 700 };
			int32_t _minHeight{ 400 };
			int32_t _maxWidth{ 3840 };
			int32_t _maxHeight{ 2160 };
		
			events::EventManager* _eventManager{ nullptr };
	};
}

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
