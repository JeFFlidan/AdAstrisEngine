#pragma once

#include <imgui/imgui.h>

namespace ad_astris::uicore
{
	class DockingWindow
	{
		public:
			DockingWindow() = default;
			~DockingWindow() = default;

			void draw_window(void* data = nullptr);
			void set_window_size(float x, float y) { _windowSize = ImVec2(x, y); }
		
		private:
			ImVec2 _windowSize;
	};
}
