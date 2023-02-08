#pragma once

#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include "proj_settings_window.h"
#include "properties_window.h"
#include "docking_window.h"
#include "vk_types.h"

class VulkanEngine;

namespace engine
{
	class UserInterface
	{
		public:
			UserInterface() = default;
		
			void init_ui(VulkanEngine* engine);

			void draw_ui(VulkanEngine* engine);
			void render_ui();

			void cleanup();

		private:
			ui::DockingWindow _dockingWindow;
			ui::ProjSettingsWindow _projSettingsWindow;
			ui::PropertiesWindow _propertiesWindow;
		
			void setup_dark_theme();
	};

}
