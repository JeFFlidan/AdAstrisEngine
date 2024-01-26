#include "profiler/logger.h"
#include "ui_manager.h"
#include "ui_core/utils.h"
#include "ui_core/font_manager.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

using namespace ad_astris;
using namespace devtools;
using namespace pl_impl;

UIManager::UIManager(std::string& rootPath, Config* config, acore::GlfwWindow* glfwWindow)
	: _menu(rootPath), _recentProjectsBrowser(config), _createNewGameBrowser(rootPath, config)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	_imguiConfigPath = rootPath + "/configs/ui/project_launcher_imgui.ini";
	ImGui::LoadIniSettingsFromDisk(_imguiConfigPath.c_str());
	ImGui_ImplGlfw_InitForOpenGL(glfwWindow->get_window_handle(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
	
	_dockingWindow.set_window_size(1280.0f, 720.0f);

	uicore::FontManager::load_default_fonts(rootPath);
	uicore::Utils::setup_dark_theme();
}

void UIManager::cleanup()
{
	ImGui::SaveIniSettingsToDisk(_imguiConfigPath.c_str());
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

	ImGui::PushFont(uicore::FontManager::get_default_font_size_17());

	MenuSection chosenMenuSection = _menu.draw_ui();
	bool shouldClose = false;
	switch (chosenMenuSection)
	{
		case MenuSection::RECENT_PROJECTS:
		{
			shouldClose = _recentProjectsBrowser.draw_ui();
			if (shouldClose && !_recentProjectsBrowser.check_cancel_button_pressed())
			{
				_projectInfo = _recentProjectsBrowser.get_chosen_project_info();
			}
			break;
		}
		case MenuSection::CREATE_NEW_GAME:
		{
			shouldClose = _createNewGameBrowser.draw_ui();
			if (shouldClose && !_createNewGameBrowser.check_cancel_button_pressed())
			{
				_projectInfo = _createNewGameBrowser.get_new_project_info();
			}
			break;
		}
	}

	ImGui::PopFont();

	ImGui::Render();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return shouldClose;
}
