#include "engine_core/material/materials.h"
#include "core/global_objects.h"
#include "resource_manager/resource_events.h"
#include "application_core/editor_events.h"
#include "engine.h"		// Really strange bug with this include. For now I placed it under all other includes, however, I must fix this bug properly in the future
#include "basic_systems.h"
#include "application_core/window_events.h"
#include "engine_events.h"
#include "module_objects.h"
#include "engine_core/basic_components.h"
#include "engine_core/basic_events.h"
#include "profiler/profiler.h"
#include "profiler/serializer.h"

using namespace ad_astris::engine::impl;

void Engine::init(EngineInitializationContext& engineInitContext)
{
	_mainWindow = engineInitContext.mainWindow;

	init_global_objects(engineInitContext.globalObjectContext);
	set_active_camera_delegate();
	subscribe_to_events();
	init_module_objects();
	init_local_objects();
	
	_project = std::make_unique<Project>();
	_project->load(*engineInitContext.projectInfo);
	
	init_renderer(engineInitContext);

	_project->postload(*engineInitContext.projectInfo);

	pre_update();
	_renderer->bake();
	LOG_INFO("Engine::init(): Baked renderer")

	LOG_INFO("Engine::init(): Engine initialization completed")
}

void Engine::execute()
{
	profiler::Profiler::begin_cpu_frame();
	_resourceLoader->load_new_resources();
	_engineObjectsCreator->create_new_objects();
	pre_update();
	SYSTEM_MANAGER()->execute();
	renderer::DrawContext drawContext;
	drawContext.activeCamera = _activeCamera;
	drawContext.deltaTime = 0.0f;	// TODO
	_renderer->draw(drawContext);
	profiler::Profiler::end_frame();
}

void Engine::save_and_cleanup(bool needToSave)
{
	SYSTEM_MANAGER()->cleanup();
	_renderer->cleanup();
	if (needToSave)
	{
		RESOURCE_MANAGER()->save_resources();
		_project->save();
	}
}

void Engine::init_global_objects(GlobalObjectContext* context)
{
	GlobalObjects::set_global_object_context(context);
	
	GlobalObjects::init_task_composer();
	LOG_INFO("Engine::init(): Initialized TaskComposer")
	
	GlobalObjects::init_resource_manager();
	LOG_INFO("Engine::init(): Initialized ResourceManager")

	GlobalObjects::init_world();
	LOG_INFO("Engine::init(): Initialized World")

	GlobalObjects::init_system_manager();
	LOG_INFO("Engine::init(): Initialized SystemManager")

	register_ecs_objects();
	LOG_INFO("Engine::init(): Registered ECS objects")
	SYSTEM_MANAGER()->generate_execution_order();
	SYSTEM_MANAGER()->add_entity_manager(WORLD()->get_entity_manager());

	LOG_INFO("Engine::init(): Generated SystemManager execution order")

	GlobalObjects::init_profiler_instance();
	profiler::Profiler::init(PROFILER_INSTANCE());
	LOG_INFO("Engine::init(): Initialized Profiler.")
}

void Engine::init_renderer(EngineInitializationContext& engineInitContext)
{
	renderer::RendererInitializationContext rendererInitContext;
	rendererInitContext.globalObjectContext = GlobalObjects::get_global_object_context();
	rendererInitContext.mainWindow = _mainWindow;
	rendererInitContext.projectSettings = _project->get_settings();
	auto rendererModule = MODULE_MANAGER()->load_module<renderer::IRendererModule>("Renderer");
	_renderer = rendererModule->get_renderer();
	_renderer->init(rendererInitContext);
	engineInitContext.uiBackendCallbacks = rendererInitContext.uiBackendCallbacks;
	PROFILER_INSTANCE()->set_rhi(_renderer->get_rhi());
	LOG_INFO("Engine::init(): Loaded and initialized Renderer module")
}

void Engine::init_module_objects()
{
	ModuleObjects::init_engine_object_creators();
	LOG_INFO("Engine::init(): Initialized engine object creators")
}

void Engine::init_local_objects()
{
	_engineObjectsCreator = std::make_unique<EngineObjectsCreator>();
	LOG_INFO("Engine::init(): Initialized EngineObjectsCreator")

	_resourceLoader = std::make_unique<ResourceLoader>();
	LOG_INFO("Engine::init(): Initialized ResourceLoader")
}

void Engine::register_ecs_objects()
{
	ecore::register_basic_components(WORLD()->get_entity_manager(), ECS_UI_MANAGER());
	SYSTEM_MANAGER()->register_system<TransformUpdateSystem>();
	SYSTEM_MANAGER()->register_system<CameraUpdateSystem>();
}

void Engine::pre_update()
{
	ecore::CameraSetEvent cameraSetEvent(_activeCamera, ecore::MAIN_CAMERA);
	EVENT_MANAGER()->trigger_event(cameraSetEvent);
	UpdateActiveCameraEvent event(_activeCamera);
	EVENT_MANAGER()->trigger_event(event);
}

void Engine::set_active_camera_delegate()
{
	events::EventDelegate<ecore::EntityCreatedEvent> _activeCameraDelegate = [this](ecore::EntityCreatedEvent& event)
	{
		ecs::Entity entity = event.get_entity();
		if (entity.has_component<ecore::CameraComponent>())
		{
			auto camera = entity.get_component<ecore::CameraComponent>();
			if (camera->isActive)
				_activeCamera = entity;
		}
	};
	EVENT_MANAGER()->subscribe(_activeCameraDelegate);
}

void Engine::subscribe_to_events()
{
	events::EventDelegate<acore::ProjectSavingStartEvent> delegate1 = [&](acore::ProjectSavingStartEvent& event)
	{
		RESOURCE_MANAGER()->save_resources();
	};
	EVENT_MANAGER()->subscribe(delegate1);
}
