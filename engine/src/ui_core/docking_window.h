#pragma once

#include "common.h"
#include <imgui/imgui.h>

namespace ad_astris::uicore
{
	class DockingWindow : public IUIWindow
	{
		public:
			DockingWindow() = default;
			~DockingWindow() = default;

			virtual void draw_window(void* data = nullptr) override;
			void set_window_size(float x, float y) { _windowSize = ImVec2(x, y); }
		
		private:
			ImVec2 _windowSize;
	};
}
