#include "engine_core/material/materials.h"
#include "resource_manager/resource_events.h"
#include "engine_core/fwd.h"
#include "application_core/editor_events.h"
#include "engine.h"		// Really strange bug with this include. For now I placed it under all other includes, however, I must fix this bug properly in the future
#include "basic_systems.h"
#include "application_core/window_events.h"
#include "engine_events.h"
#include "engine_core/basic_components.h"
#include "engine_core/basic_events.h"
#include "ui_core/ecs_ui_manager.h"
#include "engine_core/material/materials.h"

using namespace ad_astris::engine::impl;
#define GET_ID(x) compile_time_fnv1(#x)

void Engine::init(EngineInitializationContext& engineInitContext)
{
	_fileSystem = engineInitContext.fileSystem;
	_moduleManager = engineInitContext.moduleManager;
	_eventManager = engineInitContext.eventManager;
	_mainWindow = engineInitContext.mainWindow;

	subscribe_to_events();

	_taskComposer = std::make_unique<tasks::TaskComposer>();
	LOG_INFO("Engine::init(): Initialized task composer")
	
	_resourceManager = std::make_unique<resource::ResourceManager>(_fileSystem, _eventManager);
	LOG_INFO("Engine::init(): Initialized resource manager")

	ecs::create_type_info_table();
	
	ecore::WorldCreationContext worldCreationContext;
	worldCreationContext.eventManager = _eventManager;
	worldCreationContext.taskComposer = _taskComposer.get();
	_world = std::make_unique<ecore::World>(worldCreationContext);
	_ecsUIManager = std::make_unique<uicore::ECSUiManager>(_world->get_entity_manager(), ecs::get_active_type_info_table());
	LOG_INFO("Engine::init(): Initialized world")

	ecs::EngineManagers managers;
	managers.eventManager = _eventManager;
	managers.resourceManager = _resourceManager.get();
	managers.taskComposer = _taskComposer.get();
	managers.entityManager = _world->get_entity_manager();
	_systemManager = std::make_unique<ecs::SystemManager>();
	_systemManager->init(managers);
	LOG_INFO("Engine::init(): Initialized system manager")
	register_ecs_objects();
	_systemManager->generate_execution_order();
	_systemManager->add_entity_manager(_world->get_entity_manager());
	LOG_INFO("Engine::init(): Generated execution order")
	
	_engineObjectsCreator = std::make_unique<EngineObjectsCreator>(_world.get());
	LOG_INFO("Engine::init(): Initialized engine objects creator")

	switch (engineInitContext.projectInfo->newProjectTemplate)
	{
		case devtools::NewProjectTemplate::OLD_PROJECT:
		{
			load_existing_project();
			LOG_INFO("Engine::init(): Loaded an existing project {}", _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>()->get_project_name())
			break;
		}
		case devtools::NewProjectTemplate::BLANK:
		{
			create_new_blank_project();
			LOG_INFO("Engine::init(): Created a blank project {}", _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>()->get_project_name())
			break;
		}
	}

	renderer::RendererInitializationContext rendererInitContext;
	rendererInitContext.eventManager = _eventManager;
	rendererInitContext.mainWindow = _mainWindow;
	rendererInitContext.moduleManager = _moduleManager;
	rendererInitContext.projectSettings = _projectSettings.get();
	rendererInitContext.resourceManager = _resourceManager.get();
	rendererInitContext.taskComposer = _taskComposer.get();
	rendererInitContext.world = _world.get();
	rendererInitContext.typeInfoTable = ecs::get_active_type_info_table();
	auto rendererModule = _moduleManager->load_module<renderer::IRendererModule>("Renderer");
	_renderer = rendererModule->get_renderer();
	_renderer->init(rendererInitContext);
	engineInitContext.uiBackendCallbacks = rendererInitContext.uiBackendCallbacks;
	LOG_INFO("Engine::init(): Loaded and initialized Renderer module")

	if (engineInitContext.projectInfo->newProjectTemplate == devtools::NewProjectTemplate::OLD_PROJECT)
	{
		_resourceManager->load_builtin_resources();
		LOG_INFO("Engine::init(): Loaded builtin resources")
	}
	else
	{
		//create_material_templates();
		LOG_INFO("Engine::init(): Created material templates for new project")
	}

	_renderer->bake();
	LOG_INFO("Engine::init(): Baked renderer")

	LOG_INFO("Engine::init(): Engine initialization completed")

	set_active_camera_delegate();
	//_eventManager->unsubscribe(ecore::EntityCreatedEvent::get_type_id_static(), _activeCameraDelegate.target_type().name());
}

void Engine::execute()
{
	create_new_resources();
	pre_update();
	_systemManager->execute();
	renderer::DrawContext drawContext;
	drawContext.activeCamera = _activeCamera;
	drawContext.deltaTime = 0.0f;	// TODO
	_renderer->draw(drawContext);
}

void Engine::save_and_cleanup(bool needToSave)
{
	_systemManager->cleanup();
	_renderer->cleanup();
	if (needToSave)
	{
		_resourceManager->save_resources();
		_projectSettings->serialize(_fileSystem);
	}
}

void Engine::create_new_blank_project()
{
	io::URI path = _fileSystem->get_project_root_path() + "/content/levels/base_level.aalevel";
	ecore::LevelHandle level = _resourceManager->create_level(path);
	_world->add_level(level.get_resource(), true, true);
	LOG_INFO("Engine::init(): Created new level")

	ecore::DefaultSettingsContext<ecore::ProjectSettings> defaultSettingsContext;
	io::URI relativePathToDefaultLevel = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), path);
	io::Utils::replace_back_slash_to_forward(relativePathToDefaultLevel);
	defaultSettingsContext.defaultLevelPath = relativePathToDefaultLevel;
	io::URI aaprojectFilePath = io::Utils::find_file_with_specific_extension(_fileSystem->get_project_root_path(), ".aaproject");
	defaultSettingsContext.projectName = io::Utils::get_file_name(aaprojectFilePath);
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->setup_default_settings(defaultSettingsContext);
	_projectSettings->deserialize(_fileSystem->get_project_root_path() + "/configs/project_settings.ini");
	_projectSettings->serialize(_fileSystem);

	LOG_INFO("BEFORE CREATING DEFAULT MATERIAL")
	create_default_material();
	
	ecore::EditorObjectCreationContext editorObjectCreationContext;
	editorObjectCreationContext.uuid = 	_resourceManager->convert_to_aares<ecore::StaticModel>(_fileSystem->get_engine_root_path() + "/starter_content/models/plane.obj", "content").get_resource()->get_uuid();
	LOG_INFO("RESOURCE UUID: {}", (uint64_t)editorObjectCreationContext.uuid)
	editorObjectCreationContext.location = XMFLOAT3(0.0f, 0.0f, 0.0f);
	editorObjectCreationContext.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	editorObjectCreationContext.materialUUID = get_default_material_uuid();
	LOG_INFO("MATERIAL UUID: {}", get_default_material_uuid())
	ecs::Entity entity = _engineObjectsCreator->create_static_model(editorObjectCreationContext);
	auto modelComponent = _world->get_entity_manager()->get_component<ecore::ModelComponent>(entity);
	LOG_INFO("CREATED STATIC MODEL, {}", modelComponent->modelUUID)
	
	ecore::EditorObjectCreationContext creationContext{ };
	creationContext.location = XMFLOAT3(0.0f, 35.0f, 0.0f);
	_activeCamera = _engineObjectsCreator->create_camera(creationContext);
	auto cameraComponent = _world->get_entity_manager()->get_component<ecore::CameraComponent>(_activeCamera);
	cameraComponent->isActive = true;

	_engineObjectsCreator->create_directional_light(creationContext);
	creationContext.location = XMFLOAT3(12.0f, 100.0f, 25.0f);
	ecs::Entity pointLight = _engineObjectsCreator->create_point_light(creationContext);

	_resourceManager->save_resources();
	
	LOG_INFO("Engine::init(): Saved project with default settings")
}

void Engine::load_existing_project()
{
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->deserialize(_fileSystem->get_project_root_path() + "/configs/project_settings.ini");
	auto projectDescriptionSubsettings = _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>();
	io::URI levelRelativePath = projectDescriptionSubsettings->get_default_level_path().c_str();
	io::URI levelPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), levelRelativePath);
	ecore::LevelHandle level = _resourceManager->load_level(levelPath);
	_world->add_level(level.get_resource(), true, true);
	
	level.get_resource()->build_entities();
	
	LOG_INFO("Engine::init(): Loaded default level {}", level.get_resource()->get_name()->get_full_name())
}

void Engine::register_ecs_objects()
{
	ecore::register_basic_components(_world->get_entity_manager(), _ecsUIManager.get());
	_systemManager->register_system<TransformUpdateSystem>();
	_systemManager->register_system<CameraUpdateSystem>();
}

void Engine::pre_update()
{
	UpdateActiveCameraEvent event(_activeCamera);
	_eventManager->trigger_event(event);
}

void Engine::set_active_camera_delegate()
{
	events::EventDelegate<ecore::EntityCreatedEvent> _activeCameraDelegate = [this](ecore::EntityCreatedEvent& event)
	{
		ecs::Entity entity = event.get_entity();
		if (event.get_entity_manager()->does_entity_have_component<ecore::CameraComponent>(entity))
		{
			auto camera = event.get_entity_manager()->get_component<ecore::CameraComponent>(entity);
			if (camera->isActive)
				_activeCamera = entity;
		}
	};
	_eventManager->subscribe(_activeCameraDelegate);
}

void Engine::create_default_material()
{
	tasks::TaskGroup taskGroup;
	ecore::Texture2D* albedo, *normal, *roughness, *metallic, *ao;
	
	std::vector<io::URI> paths = {
		_fileSystem->get_engine_root_path() + "/starter_content/textures/DustyCobbleAlbedo.tga",
		_fileSystem->get_engine_root_path() + "/starter_content/textures/DustyCobbleAO.tga",
		_fileSystem->get_engine_root_path() + "/starter_content/textures/DustyCobbleNormal.tga",
		_fileSystem->get_engine_root_path() + "/starter_content/textures/DustyCobbleRoughness.tga",
		_fileSystem->get_engine_root_path() + "/starter_content/textures/BlackMetallic.tga",
	};
	
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		albedo = _resourceManager->convert_to_aares<ecore::Texture2D>(paths[0], "content").get_resource();
	});
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		ao = _resourceManager->convert_to_aares<ecore::Texture2D>(paths[1], "content").get_resource();
	});
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		normal = _resourceManager->convert_to_aares<ecore::Texture2D>(paths[2], "content").get_resource();
	});
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		roughness = _resourceManager->convert_to_aares<ecore::Texture2D>(paths[3], "content").get_resource();
	});
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		metallic = _resourceManager->convert_to_aares<ecore::Texture2D>(paths[4], "content").get_resource();
	});
	_taskComposer->wait(taskGroup);
	
	resource::FirstCreationContext<ecore::OpaquePBRMaterial> creationContext;
	creationContext.materialName = "DefaultOpaqueMaterial";
	creationContext.materialPath = _fileSystem->get_project_root_path() + "/content";
	creationContext.materialSettings.metallicTextureUUID = metallic->get_uuid();
	creationContext.materialSettings.baseColorTextureUUID = albedo->get_uuid();
	creationContext.materialSettings.roughnessTextureUUID = roughness->get_uuid();
	creationContext.materialSettings.normalTextureUUID = normal->get_uuid();
	creationContext.materialSettings.ambientOcclusionTextureUUID = ao->get_uuid();
	auto material = _resourceManager->create_new_resource(creationContext).get_resource();
	_projectSettings->get_subsettings<ecore::RendererSubsettings>()->set_default_material_uuid(material->get_uuid());
}

ad_astris::UUID Engine::get_default_material_uuid()
{
	return _projectSettings->get_subsettings<ecore::RendererSubsettings>()->get_default_material_uuid();
}

void Engine::subscribe_to_events()
{
	events::EventDelegate<acore::ResourceImportEvent> delegate1 = [&](acore::ResourceImportEvent& event)
	{
		_resourcePaths.push_back({ event.get_resource_path(), event.get_aares_path() });
	};
	_eventManager->subscribe(delegate1);

	events::EventDelegate<acore::ProjectSavingStartEvent> delegate2 = [&](acore::ProjectSavingStartEvent& event)
	{
		_resourceManager->save_resources();
	};
	_eventManager->subscribe(delegate2);

	events::EventDelegate<acore::OpaquePBRMaterialCreationEvent> delegate3 = [&](acore::OpaquePBRMaterialCreationEvent& event)
	{
		resource::FirstCreationContext<ecore::OpaquePBRMaterial> firstCreationContext;
		firstCreationContext.materialName = event.get_material_name();
		firstCreationContext.materialPath = event.get_material_path();
		firstCreationContext.materialSettings.baseColorTextureUUID = event.get_albedo_texture_uuid();
		firstCreationContext.materialSettings.normalTextureUUID = event.get_normal_texture_uuid();
		firstCreationContext.materialSettings.roughnessTextureUUID = event.get_roughness_texture_uuid();
		firstCreationContext.materialSettings.metallicTextureUUID = event.get_metallic_texture_uuid();
		firstCreationContext.materialSettings.ambientOcclusionTextureUUID = event.get_ao_texture_uuid();
		_materialsToCreate.push_back(firstCreationContext);
	};
	_eventManager->subscribe(delegate3);

	events::EventDelegate<acore::EditorPointLightCreationEvent> delegate4 = [&](acore::EditorPointLightCreationEvent& event)
	{
		ecore::EditorObjectCreationContext& objectCreationContext = _pointLightsToCreate.emplace_back();
		objectCreationContext.location = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};
	_eventManager->subscribe(delegate4);

	events::EventDelegate<acore::EditorStaticModelCreationEvent> delegate5 = [&](acore::EditorStaticModelCreationEvent& event)
	{
		ecore::EditorObjectCreationContext& objectCreationContext = _staticModelsToCreate.emplace_back();
		objectCreationContext.materialUUID = event.get_material_uuid();
		objectCreationContext.uuid = event.get_model_uuid();
	};
	_eventManager->subscribe(delegate5);
}

void Engine::create_new_resources()
{
	tasks::TaskGroup& taskGroup = *_taskComposer->allocate_task_group();
	for (auto& paths : _resourcePaths)
	{
		_taskComposer->execute(taskGroup, [this, paths](tasks::TaskExecutionInfo execInfo)
		{
			io::URI resourcePath = paths.first;
			io::URI aaresPath = paths.second;
			std::string fileExtension = io::Utils::get_file_extension(resourcePath);
			if (fileExtension == "jpg" || fileExtension == "tga" || fileExtension == "png")
				_resourceManager->convert_to_aares<ecore::Texture2D>(resourcePath, aaresPath).get_resource();
			if (fileExtension == "obj")
				_resourceManager->convert_to_aares<ecore::StaticModel>(resourcePath, aaresPath).get_resource();
		});
	}
	_taskComposer->wait(taskGroup);
	_taskComposer->free_task_group(&taskGroup);
	_resourcePaths.clear();

	for (auto& newMaterial : _materialsToCreate)
	{
		_resourceManager->create_new_resource(newMaterial);
	}
	if (!_materialsToCreate.empty())
	{
		_resourceManager->save_resources();
	}
	_materialsToCreate.clear();

	for (auto& context : _pointLightsToCreate)
		_engineObjectsCreator->create_point_light(context);

	for (auto& context : _staticModelsToCreate)
		_engineObjectsCreator->create_static_model(context);

	_pointLightsToCreate.clear();
	_staticModelsToCreate.clear();
}
