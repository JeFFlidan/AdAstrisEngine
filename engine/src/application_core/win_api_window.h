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
			WinApiWindow(WindowCreationContext& creationContext, events::EventManager* eventManager);
			~WinApiWindow();
			WinApiWindow(const WinApiWindow&) = delete;
			WinApiWindow& operator=(const WinApiWindow&) = delete;

			virtual bool process_messages() override;
			virtual void close() override;
			virtual HWND get_hWnd() override
			{
				return _hWnd;
			}

			virtual uint32_t get_width() override { return _width; }
			virtual uint32_t get_height() override { return _height; }
			void set_width(uint32_t width) { _width = width; }
			void set_height(uint32_t height) { _height = height; }

			void parse_keys(KeyEvent& keyEvent, WPARAM wParam);
			void setup_mouse_button_down_up_event(MouseButtonEvent* event, MouseButton button);
			void setup_mouse_move_with_pressed_button_event(MouseMoveWithPressedButtonEvent& mouseMoveButtonEvent);
			POINT get_cursor_coords_relative_to_window();

			events::EventManager* get_event_manager()
			{
				return _eventManager;
			}

			virtual bool is_running() override { return _isRunning; }

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
			uint32_t _width;
			uint32_t _height;
			bool _isRunning{ true };
		
			events::EventManager* _eventManager{ nullptr };
	};
}

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
