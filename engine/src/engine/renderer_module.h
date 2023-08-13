﻿#pragma once

#include "engine_core/project_settings/project_settings.h"
#include "resource_manager/resource_manager.h"
#include "application_core/window.h"
#include "core/module_manager.h"

namespace ad_astris::renderer
{
	class IRenderer
	{
		public:
			virtual ~IRenderer() { }
		
			virtual void init(
				ModuleManager* moduleManager,
				resource::ResourceManager* resourceManager,
				acore::IWindow* window,
				ecore::ProjectSettings& projectSettings) = 0;
			virtual void cleanup() = 0;

			virtual void bake() = 0;
	};

	class IRendererModule : public IModule
	{
		public:
			virtual IRenderer* get_renderer() = 0;
	};
}