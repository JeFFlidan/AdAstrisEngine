#pragma once

#include "engine_core/project_settings/project_settings.h"
#include "resource_manager/resource_manager.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "application_core/window.h"
#include "core/module_manager.h"
#include "rhi/ui_window_backend.h"
#include "engine_core/world.h"

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
		ecore::World* world;
		ecs::TypeInfoTable* typeInfoTable;
	};

	struct DrawContext
	{
		ecs::Entity activeCamera;
		float deltaTime;
	};
	
	class IRenderer
	{
		public:
			virtual ~IRenderer() { }
		
			virtual void init(RendererInitializationContext& initializationContext) = 0;
			virtual void cleanup() = 0;

			virtual void set_new_world(ecore::World* world) = 0;

			virtual void bake() = 0;
			virtual void draw(DrawContext& drawContext) = 0;
	};

	class IRendererModule : public IModule
	{
		public:
			virtual IRenderer* get_renderer() = 0;
	};
}