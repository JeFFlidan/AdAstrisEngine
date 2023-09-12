#include "ad_astris_engine.h"
#include "project_launcher_module.h"
#include "file_system/IO.h"
#include "application_core/win_api_window.h"

#include <filesystem>

using namespace ad_astris::app;

bool AdAstrisEngine::init()
{
	_eventManager = std::make_unique<events::EventManager>();
	LOG_INFO("AdAstrisEngine::init(): Initialized event manager")
	
	std::string _projectPath = std::filesystem::current_path().string();
	_projectPath.erase(_projectPath.find("\\bin"), 4);
	_fileSystem = std::make_unique<io::EngineFileSystem>(_projectPath.c_str());
	LOG_INFO("AdAstrisEngine::init(): Initialized file system. Engine root path is {}", _fileSystem->get_engine_root_path().c_str())

	_moduleManager = std::make_unique<ModuleManager>(_fileSystem.get());
	LOG_INFO("AdAstrisEngine::init(): Initialized module manager.")

	auto projectLauncherModule = _moduleManager->load_module<devtools::IProjectLauncherModule>("ProjectLauncher");
	auto projectLauncher = projectLauncherModule->get_project_launcher();
	projectLauncher->init(_fileSystem.get());
	LOG_INFO("AdAstrisEngine::init(): Loaded and initialized ProjectLauncher module");

	projectLauncher->draw_window();

	devtools::ProjectInfo projectInfo = projectLauncher->get_project_info();
	LOG_INFO("AdAstrisEngine::init(): Got project info from project launcher")

	_moduleManager->unload_module("ProjectLauncher");
	LOG_INFO("AdAstrisEngine::init(): Unload ProjectLauncher module")

	_moduleManager->load_module("VulkanRHI");
	LOG_INFO("AFTER VULKAN RHI")
	_moduleManager->load_module("RenderCore");
	LOG_INFO("AFTER RENDER CORE")
	_moduleManager->load_module("Renderer");
	LOG_INFO("AFTER RENDERER")
	_moduleManager->load_module("Engine");
	LOG_INFO("AFTER ENGINE")

	if (projectInfo.projectPath.empty())
	{
		LOG_INFO("AdAstrisEngine::init(): No project was chosen. Engine execution finished")
		return false;
	}

	std::string& fullProjectPath = projectInfo.projectPath;
	std::string projectName = fullProjectPath.substr(fullProjectPath.find_last_of("/") + 1, fullProjectPath.find_last_of('.') - (fullProjectPath.find_last_of("/") + 1));
	std::string projectPathWithNoFile = fullProjectPath.substr(0, fullProjectPath.find_last_of("/"));
	_fileSystem->set_project_root_path(projectPathWithNoFile.c_str());

	acore::WindowCreationContext windowCreationContext;
	windowCreationContext.width = 1280;
	windowCreationContext.height = 720;
	windowCreationContext.isResizable = true;
	windowCreationContext.windowTitle = projectName;
	_mainWindow = std::make_unique<acore::impl::WinApiWindow>(windowCreationContext, _eventManager.get());
	LOG_INFO("AdAstrisEngine::init(): Initialized WinApi main window. Window title is {}", projectName)

	engine::EngineInitializationContext engineInitializationContext;
	engineInitializationContext.eventManager = _eventManager.get();
	engineInitializationContext.fileSystem = _fileSystem.get();
	engineInitializationContext.mainWindow = _mainWindow.get();
	engineInitializationContext.moduleManager = _moduleManager.get();
	engineInitializationContext.projectInfo = &projectInfo;
	auto engineModule = _moduleManager->load_module<engine::IEngineModule>("Engine");
	_engine = engineModule->get_engine();
	_engine->init(engineInitializationContext);
	LOG_INFO("AdAstrisEngine::init(): Loaded and initalized Engine module")

	return true;
}

void AdAstrisEngine::execute()
{
	bool running = true;

	while (running)
	{
		running = _mainWindow->process_messages();
		_engine->execute();
		_eventManager->dispatch_events();
	}
	_mainWindow->close();
}

void AdAstrisEngine::save_and_cleanup()
{
	_engine->save_and_cleanup(true);
}
