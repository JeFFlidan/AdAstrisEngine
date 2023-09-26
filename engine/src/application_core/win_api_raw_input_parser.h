#pragma once

#include "core/linear_allocator.h"
#include "window_events.h"
#include <vector>
#include <windows.h>

namespace ad_astris::acore::impl
{
	class WinApiRawInputParser
	{
		public:
			void init();
			void read_raw_inputs(InputEvent& inputEvent);

		private:
			LinearAllocator _allocator;
			MouseState _mouseState;

			void parse_raw_input_buffer(const RAWINPUT& rawInput);
	};
}