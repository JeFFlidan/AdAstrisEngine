#pragma once

#include <imgui.h>
#include <string>

namespace ad_astris::uicore
{
	class Utils
	{
		public:
			static void setup_dark_theme();
			static ImVec4 get_default_fill_color();
			static ImVec4 get_default_hover_color();
			static ImVec4 get_normalized_color(float r, float g, float b, float a);
	};
}