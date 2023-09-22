#pragma once

#include "engine_core/project_settings/project_settings.h"
#include "resource_manager/resource_manager.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "application_core/window.h"
#include "core/module_manager.h"
#include "rhi/ui_window_backend.h"

namespace ad_astris::renderer
{
	struct RendererInitializationContext
	{
		ModuleManager* moduleManager;
		tasks::TaskComposer* taskComposer;
		ecore::ProjectSettings* projectSettings;
		resource::ResourceManager* resourceManager;
		events::EventManager* eventManager;
		acore::IWindow* mainWindow;
		rhi::UIWindowBackendCallbacks uiBackendCallbacks;
	};
	
	class IRenderer
	{
		public:
			virtual ~IRenderer() { }
		
			virtual void init(RendererInitializationContext& initializationContext) = 0;
			virtual void cleanup() = 0;

			virtual void bake() = 0;
			virtual void draw() = 0;
	};

	class IRendererModule : public IModule
	{
		public:
			virtual IRenderer* get_renderer() = 0;
	};
}