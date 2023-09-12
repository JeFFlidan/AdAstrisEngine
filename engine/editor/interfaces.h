#pragma once

#include <imgui/imgui.h>

namespace ad_astris::ui
{
	class IProperties
	{
		public:
			virtual ~IProperties() {}
			virtual void draw_ui() = 0;
	};

	class IWindow
	{
		public:
			virtual ~IWindow() {}
			virtual void draw_window(void* data = nullptr) = 0;
		
		private:
			virtual void draw_ui(void* data = nullptr) = 0;
	};
}
