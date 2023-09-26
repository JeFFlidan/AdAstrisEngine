#include "win_api_raw_input_parser.h"
#include "win_api_utils.h"

using namespace ad_astris;
using namespace acore;
using namespace impl;

constexpr uint32_t MAX_KEY_COUNT = 256;

void WinApiRawInputParser::init()
{
	RAWINPUTDEVICE devices[2] = { };

	// Mouse
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage = 0x02;
	devices[0].dwFlags = 0;
	devices[0].hwndTarget = 0;

	// Keyboard
	devices[1].usUsagePage = 0x01;
	devices[1].usUsage = 0x06;
	devices[1].dwFlags = 0;
	devices[1].hwndTarget = 0;

	if (RegisterRawInputDevices(devices, sizeof(devices) / sizeof(devices[0]), sizeof(devices[0])) == FALSE)
		assert(0);

	_allocator.reserve(1024 * 1024, 8);
}

void WinApiRawInputParser::read_raw_inputs(InputEvent& inputEvent)
{
	// UINT deviceCount;
	// UINT result = GetRawInputDeviceList(NULL, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	// assert(result == 0);
	//
	// static RAWINPUTDEVICELIST deviceList[64];
	// result = GetRawInputDeviceList(deviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	// assert(deviceCount <= sizeof(deviceList) / sizeof(deviceList[0]));

	while (true)
	{
		UINT rawBufferSize = 0;
		UINT result = GetRawInputBuffer(NULL, &rawBufferSize, sizeof(RAWINPUTHEADER));
		assert(result == 0);
		rawBufferSize *= 8;
		if (rawBufferSize == 0)
		{
			inputEvent.set_mouse_state(_mouseState);
			_mouseState = MouseState();
			_allocator.reset();
			return;
		}

		PRAWINPUT rawBuffer = (PRAWINPUT)_allocator.allocate((size_t)rawBufferSize);
		assert(rawBuffer != nullptr);

		UINT count = GetRawInputBuffer(rawBuffer, &rawBufferSize, sizeof(RAWINPUTHEADER));
		if (count == -1)
		{
			HRESULT error = HRESULT_FROM_WIN32(GetLastError());
			assert(0);
		}

		for (UINT currentRaw = 0; currentRaw < count; ++currentRaw)
		{
			parse_raw_input_buffer(rawBuffer[currentRaw]);
		}
	}
}

void WinApiRawInputParser::parse_raw_input_buffer(const RAWINPUT& rawInput)
{
	if (rawInput.header.dwType == RIM_TYPEKEYBOARD)
	{
		RAWKEYBOARD rawKeyboard = rawInput.data.keyboard;
		if (rawKeyboard.VKey < MAX_KEY_COUNT)
		{
			if (rawKeyboard.Flags == RI_KEY_MAKE)
			{
				// TODO
			}
		}
	}
	else if (rawInput.header.dwType == RIM_TYPEMOUSE)
	{
		RAWMOUSE rawMouse = rawInput.data.mouse;
		if (rawInput.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			if (abs(rawMouse.lLastX) < 30000)
			{
				_mouseState.deltaPosition.x += (float)rawMouse.lLastX;
			}
			if (abs(rawMouse.lLastY) < 30000)
			{
				_mouseState.deltaPosition.y += (float)rawMouse.lLastY;
			}
		}

		if (rawMouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			_mouseState.pressedButton |= MouseButton::LEFT;
		}
		if (rawMouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN)
		{
			_mouseState.pressedButton |= MouseButton::RIGHT;
		}
		if (rawMouse.usButtonFlags == RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
			_mouseState.pressedButton |= MouseButton::MIDDLE;
		}
	}
}
