#include "win_api_window.h"
#include "window_events.h"
#include "editor_events.h"

#include <iostream>
#include <memory>
#include <imgui/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace ad_astris;
using namespace acore;
using namespace impl;

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	
	auto window = reinterpret_cast<WinApiWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	
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
			window->get_event_manager()->enqueue_event(resizedEvent);
			window->set_width(resizedEvent.get_width());
			window->set_height(resizedEvent.get_height());
			break;
		}
	}

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

	DWORD style = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;

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
	
	QueryPerformanceFrequency(&_ticksPerSecond);
	QueryPerformanceCounter(&_lastTickCount);
	
	_rawInputParser.init();
	subscribe_to_events();
	_cursor = GetCursor();
}

WinApiWindow::~WinApiWindow()
{
	UnregisterClass(_className, _hInstance);
}

bool WinApiWindow::process_messages()
{
	MSG msg{};
	
	InputEvent inputEvent;
	_rawInputParser.read_raw_inputs(inputEvent);

	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			LOG_INFO("FINISH WORK")
			_isRunning = false;
			return false;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	XMFLOAT2 cursorPos = get_cursor_coords_relative_to_window();
	inputEvent.set_position(cursorPos);
	inputEvent.set_viewport_state(_viewportState.isHovered);
	_eventManager->enqueue_event(inputEvent);
	
	QueryPerformanceCounter(&_currentTickCount);
	uint64_t elapsedTicks = _currentTickCount.QuadPart - _lastTickCount.QuadPart;
	uint64_t elapsedTicksMicroseconds = elapsedTicks * 1000000 / _ticksPerSecond.QuadPart;
	float deltaTime = (float)elapsedTicksMicroseconds / 1000000.0f;
	_lastTickCount = _currentTickCount;
	DeltaTimeUpdateEvent event(deltaTime);
	_eventManager->trigger_event(event);			// Maybe enqueue

	if (_viewportState.isHovered && WinApiUtils::is_key_down(VK_RBUTTON))
	{
		SetCursor(NULL);
		RECT viewportRect;
		viewportRect.left = _viewportState.viewportMin.x;
		viewportRect.top = _viewportState.viewportMin.y;
		viewportRect.right = _viewportState.viewportMax.x;
		viewportRect.bottom = _viewportState.viewportMax.y;
		ClipCursor(&viewportRect);
	}
	else
	{
		SetCursor(_cursor);
		ClipCursor(NULL);
	}

	if (WinApiUtils::is_key_down(WinApiUtils::parse_key(Key::LEFT_CTRL)) && WinApiUtils::is_key_toggle(WinApiUtils::parse_key(Key::S)))
	{
		ProjectSavingStartEvent savingEvent;
		_eventManager->trigger_event(savingEvent);
	}

	return true;
}

void WinApiWindow::close()
{
	DestroyWindow(_hWnd);
}

XMFLOAT2 WinApiWindow::get_cursor_coords_relative_to_window()
{
	POINT point;
	if (!GetCursorPos(&point) || !ScreenToClient(_hWnd, &point))
	{
		return XMFLOAT2(0.0, 0.0);
	}
	return XMFLOAT2(point.x, point.y);
}

void WinApiWindow::subscribe_to_events()
{
	events::EventDelegate<ViewportHoverEvent> delegate1 = [&](ViewportHoverEvent& event)
	{
		_viewportState = event.get_viewport_state();
	};
	_eventManager->subscribe(delegate1);
}
