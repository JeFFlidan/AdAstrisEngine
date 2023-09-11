#include "win_api_window.h"
#include "window_events.h"

#include <iostream>
#include <memory>

using namespace ad_astris::acore;
using namespace impl;

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	using namespace ad_astris;
	
	auto window = reinterpret_cast<WinApiWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	WindowEvents windowEvents;
	
	switch (uMsg)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			auto window = reinterpret_cast<WinApiWindow*>(lpCreateStruct->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			break;
		}
		// case WM_CLOSE:
		// {
		// 	DestroyWindow(hWnd);
		// 	break;
		// }
		case WM_DESTROY:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			WindowResizedEvent resizedEvent(LOWORD(lParam), HIWORD(lParam));
			window->get_event_manager()->trigger_event(resizedEvent);
			break;
		}
		case WM_KEYDOWN:
		{
			window->parse_keys(windowEvents.keyDownEvent, wParam);
			break;
		}
		case WM_KEYUP:
		{
			window->parse_keys(windowEvents.keyUpEvent, wParam);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonDownEvent, MouseButton::LEFT);
			break;
		}
		case WM_LBUTTONUP:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonUpEvent, MouseButton::LEFT);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonDownEvent, MouseButton::MIDDLE);
			break;
		}
		case WM_MBUTTONUP:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonUpEvent, MouseButton::MIDDLE);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonDownEvent, MouseButton::RIGHT);
			break;
		}
		case WM_RBUTTONUP:
		{
			window->setup_mouse_button_down_up_event(&windowEvents.mouseButtonUpEvent, MouseButton::RIGHT);
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (wParam & MK_LBUTTON)
			{
				window->setup_mouse_move_with_pressed_button_event(windowEvents.mouseMoveLeftButton);
			}
			else if (wParam & MK_RBUTTON)
			{
				window->setup_mouse_move_with_pressed_button_event(windowEvents.mouseMoveRightButton);
			}
			break;
		}
	}

	if (window)
		windowEvents.enqueue_events(window->get_event_manager());

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WinApiWindow::WinApiWindow(WindowCreationContext& creationContext, events::EventManager* eventManager)
	: _hInstance(GetModuleHandle(nullptr)), _eventManager(eventManager)
{
	_className = "WinApi Window";

	WNDCLASS wndClass{};
	wndClass.lpszClassName = _className;
	wndClass.hInstance = _hInstance;
	wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.lpfnWndProc = window_proc;

	RegisterClass(&wndClass);

	DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;

	int32_t width = creationContext.width;
	int32_t height = creationContext.height;
	
	RECT rect;
	rect.left = 100;
	rect.top = 100;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	AdjustWindowRect(&rect, style, false);

	LPVOID lpData = static_cast<LPVOID>(this);
	_hWnd = CreateWindowEx(
		0,
		_className,
		creationContext.windowTitle.c_str(),
		style,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		_hInstance,
		lpData
	);

	ShowWindow(_hWnd, SW_SHOW);
}

WinApiWindow::~WinApiWindow()
{
	UnregisterClass(_className, _hInstance);
}

bool WinApiWindow::process_messages()
{
	MSG msg{};

	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			LOG_INFO("FINISH WORK")
			return false;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void WinApiWindow::close()
{
	DestroyWindow(_hWnd);
}

void WinApiWindow::parse_keys(KeyEvent& keyEvent, WPARAM wParam)
{
	switch (wParam)
	{
		case VK_SPACE:
			keyEvent.add_key(Key::SPACE);
			break;
		case VK_TAB:
			keyEvent.add_key(Key::TAB);
			break;
		case VK_ESCAPE:
			keyEvent.add_key(Key::ESCAPE);
			break;
		case VK_RETURN:
			keyEvent.add_key(Key::ENTER);
			break;
		case VK_LEFT:
			keyEvent.add_key(Key::LEFT);
			break;
		case VK_UP:
			keyEvent.add_key(Key::UP);
			break;
		case VK_RIGHT:
			keyEvent.add_key(Key::RIGHT);
			break;
		case VK_DOWN:
			keyEvent.add_key(Key::DOWN);
			break;
		case VK_SHIFT:
			keyEvent.add_key(Key::LEFT_SHIFT);
			break;
		case VK_MENU:
			keyEvent.add_key(Key::LEFT_ALT);
			break;
		case VK_CONTROL:
			keyEvent.add_key(Key::LEFT_CTRL);
			break;
		case 0x30:
			keyEvent.add_key(Key::_0);
			break;
		case 0x31:
			keyEvent.add_key(Key::_1);
			break;
		case 0x32:
			keyEvent.add_key(Key::_2);
			break;
		case 0x33:
			keyEvent.add_key(Key::_3);
			break;
		case 0x34:
			keyEvent.add_key(Key::_4);
			break;
		case 0x35:
			keyEvent.add_key(Key::_5);
			break;
		case 0x36:
			keyEvent.add_key(Key::_6);
			break;
		case 0x37:
			keyEvent.add_key(Key::_7);
			break;
		case 0x38:
			keyEvent.add_key(Key::_8);
			break;
		case 0x39:
			keyEvent.add_key(Key::_9);
			break;
		case 0x41:
			keyEvent.add_key(Key::A);
			break;
		case 0x42:
			keyEvent.add_key(Key::B);
			break;
		case 0x43:
			keyEvent.add_key(Key::C);
			break;
		case 0x44:
			keyEvent.add_key(Key::D);
			break;
		case 0x45:
			keyEvent.add_key(Key::E);
			break;
		case 0x46:
			keyEvent.add_key(Key::F);
			break;
		case 0x47:
			keyEvent.add_key(Key::G);
			break;
		case 0x48:
			keyEvent.add_key(Key::H);
			break;
		case 0x49:
			keyEvent.add_key(Key::I);
			break;
		case 0x4A:
			keyEvent.add_key(Key::J);
			break;
		case 0x4B:
			keyEvent.add_key(Key::K);
			break;
		case 0x4C:
			keyEvent.add_key(Key::L);
			break;
		case 0x4D:
			keyEvent.add_key(Key::M);
			break;
		case 0x4E:
			keyEvent.add_key(Key::N);
			break;
		case 0x4F:
			keyEvent.add_key(Key::O);
			break;
		case 0x50:
			keyEvent.add_key(Key::P);
			break;
		case 0x51:
			keyEvent.add_key(Key::Q);
			break;
		case 0x52:
			keyEvent.add_key(Key::R);
			break;
		case 0x53:
			keyEvent.add_key(Key::S);
			break;
		case 0x54:
			keyEvent.add_key(Key::T);
			break;
		case 0x55:
			keyEvent.add_key(Key::U);
			break;
		case 0x56:
			keyEvent.add_key(Key::V);
			break;
		case 0x57:
			keyEvent.add_key(Key::W);
			break;
		case 0x58:
			keyEvent.add_key(Key::X);
			break;
		case 0x59:
			keyEvent.add_key(Key::Y);
			break;
		case 0x5A:
			keyEvent.add_key(Key::Z);
			break;
	}
}

void WinApiWindow::setup_mouse_button_down_up_event(MouseButtonEvent* event, MouseButton button)
{
	POINT cursorPosition = get_cursor_coords_relative_to_window();
	*event = MouseButtonEvent(button, cursorPosition.x, cursorPosition.y);
}

void WinApiWindow::setup_mouse_move_with_pressed_button_event(MouseMoveWithPressedButtonEvent& mouseMoveButtonEvent)
{
	POINT cursorPos = get_cursor_coords_relative_to_window();
	mouseMoveButtonEvent.set_event_is_valid();
	mouseMoveButtonEvent.set_x_position(cursorPos.x);
	mouseMoveButtonEvent.set_y_position(cursorPos.y);
}

POINT WinApiWindow::get_cursor_coords_relative_to_window()
{
	POINT point;
	if (!GetCursorPos(&point))
	{
		return POINT();
	}
	if (!ScreenToClient(_hWnd, &point))
	{
		return POINT();
	}
	return point;
}
