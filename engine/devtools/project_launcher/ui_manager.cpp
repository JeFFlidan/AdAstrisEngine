#include "profiler/logger.h"
#include "ui_manager.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace ad_astris;
using namespace devtools;
using namespace pl_impl;

UIManager::UIManager(std::string& rootPath, Config* config, acore::GlfwWindow* glfwWindow)
	: _menu(rootPath), _recentProjectsBrowser(rootPath, config), _createNewGameBrowser(rootPath, config)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	_configPath = rootPath + "/configs/ui/project_launcher_imgui.ini";
	ImGui::LoadIniSettingsFromDisk(_configPath.c_str());
	ImGui_ImplGlfw_InitForOpenGL(glfwWindow->get_window_handle(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
	
	_dockingWindow.set_window_size(1280.0f, 720.0f);
}

void UIManager::cleanup()
{
	ImGui::SaveIniSettingsToDisk(_configPath.c_str());
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool UIManager::draw_ui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	_dockingWindow.draw_window();

	MenuSection chosenMenuSection = _menu.draw_ui();
	bool shouldClose = false;
	switch (chosenMenuSection)
	{
		case MenuSection::RECENT_PROJECTS:
		{
			shouldClose = _recentProjectsBrowser.draw_ui();
			break;
		}
		case MenuSection::CREATE_NEW_GAME:
		{
			shouldClose = _createNewGameBrowser.draw_ui();
			break;
		}
	}

	ImGui::Render();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return shouldClose;
}
