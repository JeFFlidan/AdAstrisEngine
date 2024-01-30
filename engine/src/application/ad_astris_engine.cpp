﻿#include "ad_astris_engine.h"
#include "project_launcher_module.h"
#include "file_system/IO.h"
#include "application_core/win_api_window.h"

#include <filesystem>

using namespace ad_astris::app;

bool AdAstrisEngine::init()
{
	init_global_objects();
	if (!open_project()) return false;
	init_main_window();
	init_engine();

	EVENT_MANAGER()->dispatch_events();

	return true;
}

void AdAstrisEngine::execute()
{
	bool running = true;

	while (running)
	{
		running = _mainWindow->process_messages();
		if (running)
			_editor->draw();
		_engine->execute();
		EVENT_MANAGER()->dispatch_events();
	}
	_mainWindow->close();
}

void AdAstrisEngine::save_and_cleanup()
{
	_editor->cleanup();
	_engine->save_and_cleanup(true);
}

void AdAstrisEngine::init_global_objects()
{
	_globalObjectContext = std::make_unique<GlobalObjectContext>();
	GlobalObjects::set_global_object_context(_globalObjectContext.get());
	LOG_INFO("AdAstrisEngine::init(): Initialized GlobalObjectContext.")
	
	GlobalObjects::init_event_manager();
	LOG_INFO("AdAstrisEngine::init(): Initialized EventManager.")
	
	GlobalObjects::init_file_system();
	LOG_INFO("AdAstrisEngine::init(): Initialized FileSystem. Engine root path is {}", FILE_SYSTEM()->get_engine_root_path().c_str())
	
	GlobalObjects::init_module_manager();
	LOG_INFO("AdAstrisEngine::init(): Initialized ModuleManager.")
}

bool AdAstrisEngine::open_project()
{
	auto projectLauncherModule = MODULE_MANAGER()->load_module<devtools::IProjectLauncherModule>("ProjectLauncher");
	auto projectLauncher = projectLauncherModule->get_project_launcher();
	projectLauncher->init(FILE_SYSTEM());
	LOG_INFO("AdAstrisEngine::init(): Loaded and initialized ProjectLauncher module");

	projectLauncher->draw_window();

	_projectInfo = projectLauncher->get_project_info();
	LOG_INFO("AdAstrisEngine::init(): Got project info from project launcher")

	MODULE_MANAGER()->unload_module("ProjectLauncher");
	LOG_INFO("AdAstrisEngine::init(): Unload ProjectLauncher module")

	if (_projectInfo.projectPath.empty())
	{
		LOG_INFO("AdAstrisEngine::init(): No project was chosen. Engine execution finished")
		return false;
	}

	std::string& fullProjectPath = _projectInfo.projectPath;
	std::string projectPathWithNoFile = fullProjectPath.substr(0, fullProjectPath.find_last_of("/"));
	FILE_SYSTEM()->set_project_root_path(projectPathWithNoFile.c_str());

	return true;
}

void AdAstrisEngine::init_main_window()
{
	acore::WindowCreationContext windowCreationContext;
	windowCreationContext.width = 1280;
	windowCreationContext.height = 720;
	windowCreationContext.isResizable = true;
	windowCreationContext.windowTitle = _projectInfo.projectName;
	_mainWindow = std::make_unique<acore::impl::WinApiWindow>(windowCreationContext, EVENT_MANAGER());
	LOG_INFO("AdAstrisEngine::init(): Initialized WinApi main window. Window title is {}", _projectInfo.projectName)
}

void AdAstrisEngine::init_engine()
{
	engine::EngineInitializationContext engineInitializationContext;
	engineInitializationContext.mainWindow = _mainWindow.get();
	engineInitializationContext.projectInfo = &_projectInfo;
	engineInitializationContext.globalObjectContext = _globalObjectContext.get();
	auto engineModule = MODULE_MANAGER()->load_module<engine::IEngineModule>("Engine");
	_engine = engineModule->get_engine();
	_engine->init(engineInitializationContext);
	LOG_INFO("AdAstrisEngine::init(): Loaded and initalized Engine module")
	
	editor::EditorInitContext editorInitContext;
	editorInitContext.globalObjectContext = _globalObjectContext.get();
	editorInitContext.mainWindow = _mainWindow.get();
	editorInitContext.callbacks = &engineInitializationContext.uiBackendCallbacks;
	editorInitContext.ecsUiManager = ECS_UI_MANAGER();
	auto editorModule = MODULE_MANAGER()->load_module<editor::IEditorModule>("Editor");
	_editor = editorModule->get_editor();
	_editor->init(editorInitContext);
	
	ImGui::SetCurrentContext(engineInitializationContext.uiBackendCallbacks.getContextCallback());
	rhi::UIWindowBackendCallbacks::ImGuiAllocators imGuiAllocators = engineInitializationContext.uiBackendCallbacks.getImGuiAllocators();
	ImGui::SetAllocatorFunctions(imGuiAllocators.allocFunc, imGuiAllocators.freeFunc);
	LOG_INFO("AdAstrisEngine::init(): Initialized editor")
}
