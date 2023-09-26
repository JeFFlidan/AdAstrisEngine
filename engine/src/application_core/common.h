#pragma once

#include "enums.h"
#include "core/math_base.h"

namespace ad_astris::acore
{
	struct KeyboardState
	{
		Key keys{ Key::UNKNOWN };
	};

	struct MouseState
	{
		XMFLOAT2 position{ 0.0f, 0.0f };
		XMFLOAT2 deltaPosition{ 0.0f, 0.0f };
		MouseButton pressedButton{ MouseButton::UNKNOWN };
	};
}
