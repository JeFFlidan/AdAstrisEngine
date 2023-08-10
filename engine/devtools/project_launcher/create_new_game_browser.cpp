#include "create_new_game_browser.h"
#include "utils.h"
#include "ui_core/utils.h"
#include "ui_core/font_manager.h"
#include "file_system/utils.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>

using namespace ad_astris;
using namespace devtools;
using namespace pl_impl;

CreateNewGameBrowser::CreateNewGameBrowser(std::string& rootPath, Config* config) : _config(config)
{
	_projectPathInputWidget = uicore::TextInputWidget("Project Location", 625, "", true, 512);
	_projectNameInputWidget = uicore::TextInputWidget("Project Name", 165, "", true, 36);
	_fileExplorer = uicore::FileExplorer(uicore::FileExplorerMode::PICK_FOLDER);

	uicore::WidgetSelectionManagerCreationContext selectionManagerCreationContext;
	selectionManagerCreationContext.multipleSelection = false;
	selectionManagerCreationContext.permanentSelection = true;
	selectionManagerCreationContext.drawImageButtonLabel = true;
	_thumbnailTableManager = std::make_unique<uicore::ThumbnailTableManager>("Game Templates", 128.0f, selectionManagerCreationContext);

	std::vector<std::string> _projectTemplateNames = {
		"Blank",
	};

	for (auto& projectName : _projectTemplateNames)
	{
		uicore::TextureInfo textureInfo;
		Utils::load_texture_from_file(rootPath + "/engine/devtools/project_launcher/icons/" + projectName + ".png", textureInfo);
		textureInfo.width = _thumbnailTableManager->get_thumbnail_size();
		textureInfo.height = _thumbnailTableManager->get_thumbnail_size();
		bool defaultTemplate = projectName == "Blank" ? true : false;
		_thumbnailTableManager->add_button(projectName, textureInfo, defaultTemplate);
		Utils::load_texture_from_file(rootPath + "/engine/devtools/project_launcher/icons/" + projectName + "_description.png", textureInfo);
		_projectTemplateDescriptionTextureByItsName[projectName] = textureInfo;
	}
}

bool CreateNewGameBrowser::draw_ui()
{
	ImGui::PushFont(uicore::FontManager::get_default_font_size_14());
	
	ImGui::Begin("Game Presets");
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
	auto& widgetNames = _thumbnailTableManager->draw();
	std::string selectedPreset = *widgetNames.begin();
	ImGui::PopStyleVar(2);
	ImGui::End();
	ImGui::PopFont();
	
	ImGui::Begin("Preset Info");
	draw_preset_info(selectedPreset);
	ImGui::End();

	ImGui::Begin("Preset Settings");
	draw_preset_settings(selectedPreset);
	ImGui::End();
	
	ImGui::Begin("New Project Settings");
	
	bool browseButtonPressed = ImGui::Button("Browse...", ImVec2(90, 30));
	if (browseButtonPressed)
	{
		std::vector<std::string> paths;
		bool isDirPicked = _fileExplorer.open_file_explorer(paths);
		if (isDirPicked)
		{
			io::URI path = paths[0].c_str();
			io::Utils::replace_back_slash_to_forward(path);
			_projectPathInputWidget.set_text(path.c_str());
		}
	}
	ImGui::SameLine();
	_projectPathInputWidget.draw();
	_projectNameInputWidget.draw();
	
	ImGui::SetCursorPos(ImVec2(690, 125));
	ImGui::PushStyleColor(ImGuiCol_Button, uicore::Utils::get_default_fill_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uicore::Utils::get_default_hover_color());
	bool openButtonPressed = ImGui::Button("Create", ImVec2(90, 30));
	ImGui::PopStyleColor(2);
	
	ImGui::SetCursorPos(ImVec2(800, 125));
	bool cancelButtonPressed = ImGui::Button("Cancel", ImVec2(90, 30));

	ImGui::End();

	if (cancelButtonPressed)
	{
		_cancelButtonPressed = true;
		return true;
	}
	if (openButtonPressed)
	{
		if (_projectPathInputWidget.get_text().empty() || _projectNameInputWidget.get_text().empty())
			return false;
		std::string projectPath = _projectPathInputWidget.get_text();
		std::string projectName = _projectNameInputWidget.get_text();
		create_folders(projectPath, projectName);

		std::string projectNameWithAAPROJECTFile = projectPath + "/" + projectName + ".aaproject";
		Section section(projectNameWithAAPROJECTFile);
		section.set_option("Name", projectName);
		_config->set_section(section);
		_projectInfo.projectPath = projectNameWithAAPROJECTFile;
		_projectInfo.newProjectTemplate = get_enum_project_template_type(selectedPreset);
		return true;
	}
		
	return false;
}

void CreateNewGameBrowser::draw_preset_info(std::string& presetName)
{
	if (presetName == "Blank")
	{
		uicore::TextureInfo& textureInfo = _projectTemplateDescriptionTextureByItsName[presetName];
		ImVec2 size(textureInfo.width, textureInfo.height);
		ImGui::ImageButton((void*)(intptr_t)textureInfo.textureID, size, textureInfo.uv0, textureInfo.uv1);
		ImGui::PushFont(uicore::FontManager::get_default_font_size_17());
		ImGui::Text("Blank");
		ImGui::PopFont();
		ImGui::PushFont(uicore::FontManager::get_default_font_size_14());
		ImGui::Text("An empty project with no code");
		ImGui::PopFont();
	}
}

void CreateNewGameBrowser::draw_preset_settings(std::string& presetName)
{
	// TODO
}

void CreateNewGameBrowser::create_folders(std::string& projectPath, std::string& projectName)
{
	std::filesystem::create_directories(projectPath + "/icons");
	std::filesystem::create_directories(projectPath + "/configs");
	std::filesystem::create_directories(projectPath + "/content");
	std::ofstream aaprojectFile(projectPath + "/" + projectName + ".aaproject", std::ios::trunc);
}

NewProjectTemplate CreateNewGameBrowser::get_enum_project_template_type(std::string& projectTemplateName)
{
	if (projectTemplateName == "Blank")
		return NewProjectTemplate::BLANK;
	else
		return NewProjectTemplate::OLD_PROJECT;
}
