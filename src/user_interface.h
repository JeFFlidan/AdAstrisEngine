#pragma once

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
	class UserInterface
	{
		public:
			void init_ui(VulkanEngine* engine);
			void setup_dark_theme();

			void draw_ui(VulkanEngine* engine);
			void render_ui(VulkanEngine* engine);

			void cleanup();
	};

	class PointLightWindow
	{
		public:
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
			actors::PointLight* _pointLight;
	};

	class DirLightWindow
	{
		public:
			DirLightWindow(actors::DirectionLight& dirLight);

			void draw_window(VulkanEngine* engine);
			void set_dir_light_data();
		private:
			float _color[3];
			float _intensity;
			bool _isVisible;
			bool _castShadows;
			actors::DirectionLight* _dirLight;
	};

	class SpotLightWindow
	{
		public:
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
			actors::SpotLight* _spotLight;
	};

	class SettingsWindow
	{
		public:
			SettingsWindow(Settings& settings);

			void draw_window();
			void set_settings_data();

		private:
			bool _isTaaEnabled;
			Settings* _settings;
	};
}
