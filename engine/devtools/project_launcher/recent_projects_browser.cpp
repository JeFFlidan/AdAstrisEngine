#include "recent_projects_browser.h"
#include "utils.h"
#include "ui_core/file_explorer.h"
#include "ui_core/font_manager.h"
#include "ui_core/utils.h"
#include "file_system/utils.h"
#include <imgui/imgui.h>

using namespace ad_astris::devtools::pl_impl;

RecentProjectsBrowser::RecentProjectsBrowser(Config* config) : _config(config)
{
	parse_config();
	_projectPathInputWidget = uicore::TextInputWidget("Project Location", 650, "", false, 512);
	_projectNameWidget = uicore::TextInputWidget("Project Name", 165, "", false, 36);
	_fileExplorer = uicore::FileExplorer(uicore::FileExplorerMode::OPEN_FILE, { uicore::FileFilter::AAPROJECT });

	uicore::WidgetSelectionManagerCreationContext selectionManagerCreationContext;
	selectionManagerCreationContext.multipleSelection = false;
	selectionManagerCreationContext.permanentSelection = true;
	selectionManagerCreationContext.drawImageButtonLabel = true;
	_thumbnailTableManager = std::make_unique<uicore::ThumbnailTableManager>("Projects", 128.0f, selectionManagerCreationContext);

	for (auto& pair : _projectPathByItsName)
	{
		add_thumbnail(pair.second, const_cast<std::string&>(pair.first));
	}
}

bool RecentProjectsBrowser::draw_ui()
{
	ImGui::Begin("Projects");
	ImGui::PushFont(uicore::FontManager::get_default_font_size_14());
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
	auto& widgetNames = _thumbnailTableManager->draw();
	fill_text_input_widgets(widgetNames);
	ImGui::PopStyleVar(2);
	ImGui::PopFont();
	ImGui::End();
	
	ImGui::Begin("Project Info");
	
	_projectPathInputWidget.draw();
	_projectNameWidget.draw();

	ImGui::SetCursorPos(ImVec2(580, 125));
	bool browseButtonPressed = ImGui::Button("Browse...", ImVec2(90, 30));
	if (browseButtonPressed)
	{
		std::vector<std::string> paths;
		bool isFileOpened = _fileExplorer.open_file_explorer(paths);
		std::string fileName = io::Utils::get_file_name(paths[0].c_str());
		if (isFileOpened && _projectPathByItsName.find(fileName) == _projectPathByItsName.end())
		{
			io::URI path = paths[0].c_str();
			io::Utils::replace_back_slash_to_forward(path);
			_projectPathInputWidget.set_text(path.c_str());
			_projectNameWidget.set_text(fileName);
			_projectPathByItsName[fileName] = path.c_str();
			add_thumbnail(_projectPathByItsName[fileName], fileName);
		}
	}
	
	ImGui::SetCursorPos(ImVec2(690, 125));
	ImGui::PushStyleColor(ImGuiCol_Button, uicore::Utils::get_default_fill_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uicore::Utils::get_default_hover_color());
	bool openButtonPressed = ImGui::Button("Open", ImVec2(90, 30));
	ImGui::PopStyleColor(2);
	
	ImGui::SetCursorPos(ImVec2(800, 125));
	bool cancelButtonPressed = ImGui::Button("Cancel", ImVec2(90, 30));
	
	ImGui::End();
	
	_projectInfo.projectPath = _projectPathInputWidget.get_text();
	_projectInfo.projectName = _projectNameWidget.get_text();
	
	if (cancelButtonPressed || openButtonPressed)
	{
		for (auto& pair : _projectPathByItsName)
		{
			Section section(pair.second);
			Option option("Name", pair.first);
			section.set_option(option);
			_config->set_section(section);
		}

		if (cancelButtonPressed)
			_cancelButtonPressed = true;

		return true;
	}
	
	return false;
}

void RecentProjectsBrowser::parse_config()
{
	for (auto section : *_config)
	{
		io::URI path = section.get_name().c_str();
		io::Utils::replace_back_slash_to_forward(path);
		_projectPathByItsName[section.get_option_value<std::string>("Name")] = path.c_str();
	}
}

void RecentProjectsBrowser::add_thumbnail(std::string& projectPath, std::string& buttonName)
{
	std::string pathWithoutFileNames = projectPath;
	pathWithoutFileNames.erase(pathWithoutFileNames.find_last_of('/'), pathWithoutFileNames.size());
	uicore::TextureInfo textureInfo;
	Utils::load_texture_from_file(pathWithoutFileNames + "/icons/project_icon.png", textureInfo);
	textureInfo.width = _thumbnailTableManager->get_thumbnail_size();
	textureInfo.height = _thumbnailTableManager->get_thumbnail_size();
	_thumbnailTableManager->add_button(buttonName, textureInfo);
}

void RecentProjectsBrowser::fill_text_input_widgets(std::unordered_set<std::string>& widgetNames)
{
	if (!widgetNames.empty())
	{
		_projectNameWidget.set_text(*widgetNames.begin());
		_projectPathInputWidget.set_text(_projectPathByItsName[*widgetNames.begin()]);
	}
}
