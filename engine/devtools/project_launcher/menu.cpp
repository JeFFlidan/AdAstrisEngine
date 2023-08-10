#include "menu.h"
#include "utils.h"
#include "profiler/logger.h"

#include <imgui.h>

using namespace ad_astris::devtools::pl_impl;

Menu::Menu(std::string& rootPath)
{
	std::string recentProjectsTexturePath = rootPath + "/engine/devtools/project_launcher/icons/recent_projects.png";
	std::string createNewGameTexturePath = rootPath + "/engine/devtools/project_launcher/icons/create_new_game.png";

	uicore::TextureInfo recentProjectsTextureInfo;
	uicore::TextureInfo createNewGameTextureInfo;

	if (!Utils::load_texture_from_file(recentProjectsTexturePath, recentProjectsTextureInfo))
		LOG_FATAL("ProjectLauncher: Failed to load recent projects icon")

	if (!Utils::load_texture_from_file(createNewGameTexturePath, createNewGameTextureInfo))
		LOG_FATAL("ProjectLauncher: Failed to load create new game icon")

	uicore::WidgetSelectionManagerCreationContext selectionManagerCreationContext;
	selectionManagerCreationContext.multipleSelection = false;
	selectionManagerCreationContext.permanentSelection = true;
	_highlightManager = std::make_unique<uicore::WidgetSelectionManager>(selectionManagerCreationContext);

	_highlightManager->add_image_button("recent_projects", recentProjectsTextureInfo, true);
	_highlightManager->add_image_button("create_new_game", createNewGameTextureInfo);

	_lastChosenMenuSection = MenuSection::RECENT_PROJECTS;
}

MenuSection Menu::draw_ui()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Begin("Menu");
	auto& buttonNames = _highlightManager->draw_widgets();
	ImGui::PopStyleVar(1);
	ImGui::End();

	if (buttonNames.find("recent_projects") != buttonNames.end())
		_lastChosenMenuSection = MenuSection::RECENT_PROJECTS;
	else if (buttonNames.find("create_new_game") != buttonNames.end())
		_lastChosenMenuSection = MenuSection::CREATE_NEW_GAME;

	return _lastChosenMenuSection;
}
