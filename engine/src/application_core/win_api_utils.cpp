#include "win_api_utils.h"

using namespace ad_astris;
using namespace acore;
using namespace impl;

Key WinApiUtils::parse_key(WPARAM wParam)
{
	switch (wParam)
	{
		case VK_SPACE:
			return Key::SPACE;
		case VK_TAB:
			return Key::TAB;
		case VK_ESCAPE:
			return Key::ESCAPE;
		case VK_RETURN:
			return Key::ENTER;
		case VK_LEFT:
			return Key::LEFT;
		case VK_UP:
			return Key::UP;
		case VK_RIGHT:
			return Key::RIGHT;
		case VK_DOWN:
			return Key::DOWN;
		case VK_SHIFT:
			return Key::LEFT_SHIFT;
		case VK_MENU:
			return Key::LEFT_ALT;
		case VK_CONTROL:
			return Key::LEFT_CTRL;
		case 0x30:
			return Key::_0;
		case 0x31:
			return Key::_1;
		case 0x32:
			return Key::_2;
		case 0x33:
			return Key::_3;
		case 0x34:
			return Key::_4;
		case 0x35:
			return Key::_5;
		case 0x36:
			return Key::_6;
		case 0x37:
			return Key::_7;
		case 0x38:
			return Key::_8;
		case 0x39:
			return Key::_9;
		case 0x41:
			return Key::A;
		case 0x42:
			return Key::B;
		case 0x43:
			return Key::C;
		case 0x44:
			return Key::D;
		case 0x45:
			return Key::E;
		case 0x46:
			return Key::F;
		case 0x47:
			return Key::G;
		case 0x48:
			return Key::H;
		case 0x49:
			return Key::I;
		case 0x4A:
			return Key::J;
		case 0x4B:
			return Key::K;
		case 0x4C:
			return Key::L;
		case 0x4D:
			return Key::M;
		case 0x4E:
			return Key::N;
		case 0x4F:
			return Key::O;
		case 0x50:
			return Key::P;
		case 0x51:
			return Key::Q;
		case 0x52:
			return Key::R;
		case 0x53:
			return Key::S;
		case 0x54:
			return Key::T;
		case 0x55:
			return Key::U;
		case 0x56:
			return Key::V;
		case 0x57:
			return Key::W;
		case 0x58:
			return Key::X;
		case 0x59:
			return Key::Y;
		case 0x5A:
			return Key::Z;
	}
}

uint8_t WinApiUtils::parse_key(Key key)
{
	switch (key)
	{
		case Key::SPACE:
			return VK_SPACE;
		case Key::TAB:
			return VK_TAB;
		case Key::ESCAPE:
			return VK_ESCAPE;
		case Key::ENTER:
			return VK_RETURN;
		case Key::LEFT:
			return VK_LEFT;
		case Key::UP:
			return VK_UP;
		case Key::RIGHT:
			return VK_RIGHT;
		case Key::DOWN:
			return VK_DOWN;
		case Key::LEFT_SHIFT:
			return VK_SHIFT;
		case Key::LEFT_ALT:
			return VK_MENU;
		case Key::LEFT_CTRL:
			return VK_CONTROL;
		case Key::_0:
			return 0x30;
		case Key::_1:
			return 0x31;
		case Key::_2:
			return 0x32;
		case Key::_3:
			return 0x33;
		case Key::_4:
			return 0x34;
		case Key::_5:
			return 0x35;
		case Key::_6:
			return 0x36;
		case Key::_7:
			return 0x37;
		case Key::_8:
			return 0x38;
		case Key::_9:
			return 0x39;
		case Key::A:
			return 0x41;
		case Key::B:
			return 0x42;
		case Key::C:
			return 0x43;
		case Key::D:
			return 0x44;
		case Key::E:
			return 0x45;
		case Key::F:
			return 0x46;
		case Key::G:
			return 0x47;
		case Key::H:
			return 0x48;
		case Key::I:
			return 0x49;
		case Key::J:
			return 0x4A;
		case Key::K:
			return 0x4B;
		case Key::L:
			return 0x4C;
		case Key::M:
			return 0x4D;
		case Key::N:
			return 0x4E;
		case Key::O:
			return 0x4F;
		case Key::P:
			return 0x50;
		case Key::Q:
			return 0x51;
		case Key::R:
			return 0x52;
		case Key::S:
			return 0x53;
		case Key::T:
			return 0x54;
		case Key::U:
			return 0x55;
		case Key::V:
			return 0x56;
		case Key::W:
			return 0x57;
		case Key::X:
			return 0x58;
		case Key::Y:
			return 0x59;
		case Key::Z:
			return 0x5A;
	}
}

bool WinApiUtils::is_key_down(uint8_t keyCode)
{
	return GetAsyncKeyState(keyCode) < 0;
}

bool WinApiUtils::is_key_toggle(uint8_t keyCode)
{
	return (GetAsyncKeyState(keyCode) & 1) != 0;
}
