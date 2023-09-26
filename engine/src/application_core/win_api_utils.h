#pragma once

#include "enums.h"
#include "common.h"
#define NOMINMAX
#include <windows.h>

namespace ad_astris::acore::impl
{
	class WinApiUtils
	{
		public:
			static Key parse_key(WPARAM wParam);
			static uint8_t parse_key(Key key);
			static bool is_key_down(uint8_t keyCode);
			static bool is_key_toggle(uint8_t keyCode);
	};
}