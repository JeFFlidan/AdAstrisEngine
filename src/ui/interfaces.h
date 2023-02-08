#pragma once

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>

namespace engine::ui
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
