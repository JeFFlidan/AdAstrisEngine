#pragma once

#include "interfaces.h"
#include "engine_core/engine_actors.h"
#include "vulkan_renderer/vk_scene.h"

namespace ad_astris::ui
{
	class PointLightUi : public IProperties
	{
		public:
			PointLightUi(actors::PointLight* pointLight, RenderScene* renderScene);
			~PointLightUi() final = default;

			void draw_ui() final;
		private:
			actors::PointLight* _pointLight;
			RenderScene* _renderScene;
	};

	class SpotLightUi : public IProperties
	{
		public:
			SpotLightUi(actors::SpotLight* spotLight, RenderScene* renderScene);
			~SpotLightUi() final = default;
			
			void draw_ui() final;
		private:
			actors::SpotLight* _spotLight;
			RenderScene* _renderScene;
	};

	class DirLightUi : public IProperties
	{
		public:
			DirLightUi(actors::DirectionalLight* dirLight, RenderScene* renderScene);
			~DirLightUi() final = default;
			
			void draw_ui() final;
		private:
			actors::DirectionalLight* _dirLight;
			RenderScene* _renderScene;
	};
}