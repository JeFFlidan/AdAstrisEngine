#include "engine.h"
#include "engine_core/fwd.h"

using namespace ad_astris::engine::impl;

void Engine::init(EngineInitializationContext& initializationContext)
{
	_fileSystem = initializationContext.fileSystem;
	_moduleManager = initializationContext.moduleManager;
	_eventManager = initializationContext.eventManager;
	_mainWindow = initializationContext.mainWindow;
	
	_resourceManager = std::make_unique<resource::ResourceManager>(_fileSystem, _eventManager);
	LOG_INFO("Engine::init(): Initialized resource manager")

	_world = std::make_unique<ecore::World>();
	LOG_INFO("Engine::init(): Initialized world")

	switch (initializationContext.projectInfo->newProjectTemplate)
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

	//auto rendererModule = _moduleManager->load_module<renderer::IRendererModule>("Renderer");
	//_renderer = rendererModule->get_renderer();
	//_renderer->init(_moduleManager, _resourceManager.get(), window, *_engineSettings.get());

	LOG_INFO("Engine::init(): Engine initialization completed")
}

void Engine::save_and_cleanup(bool needToSave)
{
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
	LOG_INFO("Engine::init(): Saved project with default settings")
}

void Engine::load_existing_project()
{
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->deserialize(_fileSystem->get_project_root_path() + "/configs/project_settings.ini");
	LOG_INFO("Engine::init(): Loaded project settings config")
	auto projectDescriptionSubsettings = _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>();
	LOG_INFO("After loading project description ")
	io::URI levelRelativePath = projectDescriptionSubsettings->get_default_level_path().c_str();
	io::URI levelPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), levelRelativePath);
	LOG_INFO("Level path: {}", levelPath.c_str())
	ecore::LevelHandle level = _resourceManager->load_level(levelPath);
	LOG_INFO("After loading level")
	_world->add_level(level.get_resource(), true, true);
	level.get_resource()->build_entities();
	LOG_INFO("After building entities")
	LOG_INFO("Engine::init(): Loaded default level {}", level.get_resource()->get_name()->get_full_name())
}
