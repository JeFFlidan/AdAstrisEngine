#pragma once

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_vulkan.h>

#include "proj_settings_window.h"
#include "properties_window.h"
#include "docking_window.h"
#include "vulkan_renderer/vk_types.h"

class VkRenderer;

namespace ad_astris
{
	class UserInterface
	{
		public:
			UserInterface() = default;
		
			void init_ui(VkRenderer* engine);

			void draw_ui(VkRenderer* engine);
			void render_ui();

			void save_ini_file(VkRenderer* engine);

		private:
			ui::DockingWindow _dockingWindow;
			ui::ProjSettingsWindow _projSettingsWindow;
			ui::PropertiesWindow _propertiesWindow;
		
			void setup_dark_theme();
	};
}
