#include "project_launcher.h"

using namespace ad_astris;
using namespace devtools;
using namespace pl_impl;

void ProjectLauncher::init(io::FileSystem* fileSystem)
{
	create_window();
	_fileSystem = fileSystem;
	_config = std::make_unique<Config>();
	std::string rootPath(_fileSystem->get_root_path().c_str());
	io::URI configPath = (rootPath + "/configs/projects.ini").c_str(); 
	_config->load_from_file(configPath);
	_uiManager = std::make_unique<UIManager>(rootPath, _config.get(), _glfwWindow.get());
}

void ProjectLauncher::draw_window()
{
	bool shouldClose = false;
	
	while (!shouldClose)
	{
		shouldClose = _glfwWindow->process_messages();
		if (!shouldClose)
			shouldClose = _uiManager->draw_ui();
		_glfwWindow->swap_buffers();
	}
	
	_uiManager->cleanup();
	_glfwWindow->cleanup();
}

void ProjectLauncher::create_window()
{
	acore::WindowCreationContext creationContext;
	creationContext.width = 1280;
	creationContext.height = 720;
	creationContext.isResizable = true;
	creationContext.windowTitle = "AdAstris Project Launcher";
	_glfwWindow = std::make_unique<acore::GlfwWindow>(creationContext);
}
