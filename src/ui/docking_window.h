#pragma once

#include "interfaces.h"

struct Settings;

namespace ad_astris::ui
{
	class DockingWindow : public IWindow
	{
		public:
			DockingWindow() = default;
			~DockingWindow() final = default;

			void draw_window(void* data = nullptr) final;
			void set_window_size(float x, float y) { _windowSize = ImVec2(x, y); }
		private:
			ImVec2 _windowSize;
		
			void draw_ui(void* data) final;
	};
}
