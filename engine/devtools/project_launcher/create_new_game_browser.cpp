#include "create_new_game_browser.h"
#include <imgui.h>

using namespace ad_astris::devtools::pl_impl;

CreateNewGameBrowser::CreateNewGameBrowser(std::string& rootPath, Config* config) : _config(config)
{
	
}

bool CreateNewGameBrowser::draw_ui()
{
	ImGui::Begin("Game Presets");
	ImGui::End();
	ImGui::Begin("Preset Info");
	ImGui::End();
	ImGui::Begin("Preset Settings");
	ImGui::End();
	ImGui::Begin("New Project Info");
	ImGui::End();
		
	return false;
}
