#pragma once

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include "engine_actors.h"
#include "vk_types.h"
#include <vulkan/vulkan_core.h>

class VulkanEngine;
struct Settings;

namespace ui
{
	class PointLightWindow
	{
		public:
			PointLightWindow() = default;
			PointLightWindow(actors::PointLight& pointLight);

			void draw_window(VulkanEngine* engine);
			void set_point_light_data();
		private:
			float _position[3];
			float _color[3];
			float _intensity;
			float _attenuationRadius;
			float _sourceRadius;
			bool _isVisible;
			bool _castShadows;
			actors::PointLight* _pointLight{ nullptr };
	};

	class DirLightWindow
	{
		public:
			DirLightWindow() = default;
			DirLightWindow(actors::DirectionLight& dirLight);

			void draw_window(VulkanEngine* engine);
			void set_dir_light_data();
		private:
			float _color[3];
			float _intensity;
			bool _isVisible;
			bool _castShadows;
			actors::DirectionLight* _dirLight{ nullptr };
	};

	class SpotLightWindow
	{
		public:
			SpotLightWindow() = default;
			SpotLightWindow(actors::SpotLight& spotLight);

			void draw_window(VulkanEngine* engine);
			void set_spot_light_data();
		private:
			float _color[3];
			float _positioin[3];
			float _rotation[3];
			float _intensity;
			float _distance;
			float _outerConeRadius;
			float _innerConeRadius;
			bool _isVisible;
			bool _castShadows;
			actors::SpotLight* _spotLight{ nullptr };
	};

	class SettingsWindow
	{
		public:
			SettingsWindow() = default;
			SettingsWindow(Settings& settings);

			void draw_window();
			void set_settings_data();

		private:
			bool _isTaaEnabled;
			float _taaAlpha;
			Settings* _settings;
	};

	class DockingWindow
	{
		public:
			void draw_window(ImVec2 windowSize);
	};
	
	class UserInterface
	{
		public:
			UserInterface() = default;
		
			void init_ui(VulkanEngine* engine);
			void setup_dark_theme();

			void draw_ui(VulkanEngine* engine);
			void render_ui();

			void cleanup();

		private:
			DockingWindow _dockingWindow;
			SpotLightWindow _spotLightWindow;
			PointLightWindow _pointLightWindow;
			DirLightWindow _dirLightWidnow;
			SettingsWindow _settingsWindow;
	};

}
