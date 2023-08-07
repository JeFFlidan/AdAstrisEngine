#include "menu.h"
#include "profiler/logger.h"

#include <imgui.h>

using namespace ad_astris::devtools::pl_impl;

Menu::Menu(std::string& rootPath)
{
	std::string recentProjectsTexturePath = rootPath + "/engine/devtools/project_launcher/icons/recent_projects.png";
	std::string createNewGameTexturePath = rootPath + "/engine/devtools/project_launcher/icons/create_new_game.png";

	if (!Utils::load_texture_from_file(recentProjectsTexturePath, _recentProjectsTextureInfo))
		LOG_FATAL("ProjectLauncher: Failed to load recent projects icon")

	if (!Utils::load_texture_from_file(createNewGameTexturePath, _createNewGameTextureInfo))
		LOG_FATAL("ProjectLauncher: Failed to load create new game icon")

	_lastChosenMenuSection = MenuSection::RECENT_PROJECTS;
}

MenuSection Menu::draw_ui()
{
	ImGui::Begin("Menu");
	bool recentProjectsPressed = ImGui::ImageButton((void*)(intptr_t)_recentProjectsTextureInfo.textureID, ImVec2(_recentProjectsTextureInfo.width, _recentProjectsTextureInfo.height));
	bool createNewGamePressed = ImGui::ImageButton((void*)(intptr_t)_createNewGameTextureInfo.textureID, ImVec2(_createNewGameTextureInfo.width, _createNewGameTextureInfo.height));
	ImGui::End();

	if (recentProjectsPressed)
		_lastChosenMenuSection = MenuSection::RECENT_PROJECTS;
	else if (createNewGamePressed)
		_lastChosenMenuSection = MenuSection::CREATE_NEW_GAME;

	return _lastChosenMenuSection;
}
