#include "project_launcher.h"
#include <iostream>

using namespace ad_astris;
using namespace devtools;
using namespace impl;

ProjectLauncher::ProjectLauncher()
{
	create_window();
}

void ProjectLauncher::draw_window()
{
	bool shouldClose = false;
	while (!shouldClose)
	{
		shouldClose = _glfwWindow->process_messages();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		_glfwWindow->swap_buffers();
	}

	_glfwWindow->terminate();
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
