#pragma once

#include "interfaces.h"

struct Settings;

namespace engine::ui
{
	class ProjSettingsWindow : public IWindow
	{
		public:
			ProjSettingsWindow() = default;
			~ProjSettingsWindow() final = default;

			void draw_window(void* data) final;
		
		private:
			void draw_ui(void* data) final;
	};
}