#include "content_browser_window.h"
#include "resource_manager/resource_events.h"
#include "ui_core/thumbnail_table_manager.h"
#include "ui_core/file_explorer.h"
#include "application_core/editor_events.h"
#include "ui_core/utils.h"
#include "resource_manager/utils.h"
#include "core/config_base.h"
#include "file_system/utils.h"
#include <imgui/imgui.h>
#include <filesystem>

using namespace ad_astris;
using namespace editor;
using namespace impl;

ContentBrowserWindow::ContentBrowserWindow(UIWindowInitContext& initContext, rhi::GetIconsCallback getIconsCallback)
	: UIWindowInternal(initContext), _icons(getIconsCallback())
{
	_thumbnailSize = 128;

	std::vector<uicore::FileFilter> filters = {
		uicore::FileFilter::PNG,
		uicore::FileFilter::TGA,
		uicore::FileFilter::OBJ,
		uicore::FileFilter::GLTF };
	_fileExplorer = uicore::FileExplorer(uicore::FileExplorerMode::OPEN_MULTIPLE_FILES, filters);
	
	_currentPath = _fileSystem->get_project_root_path();
	std::string baseFolder = _fileSystem->get_project_root_path().string();
	baseFolder.erase(baseFolder.find_last_of("/"), baseFolder.size());
	_baseFolder = baseFolder;

	uicore::WidgetSelectionManagerCreationContext creationContext;
	creationContext.multipleSelection = true;
	creationContext.permanentSelection = false;
	creationContext.drawImageButtonLabel = true;
	creationContext.editableButtonLabel = true;
	creationContext.hoveredWidgetMouseDoubleClickedCallback = [&](uicore::internal::BaseWidget& widget)
	{
		io::URI path = _currentPath;
		path += "/" + widget.get_name();
		if (std::filesystem::is_directory(path.c_str()))
		{
			_currentPath = path;
		}
	};
	_thumbnailTableManager = std::make_unique<uicore::ThumbnailTableManager>("ContentBrowser", _thumbnailSize, creationContext);
}

void ContentBrowserWindow::draw_window(void* data)
{
	ImGui::Begin("Content Browser");

	ImGui::PushStyleColor(ImGuiCol_Button, uicore::Utils::get_default_fill_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uicore::Utils::get_default_hover_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, uicore::Utils::get_default_hover_color());
	bool isClicked = ImGui::Button("Import");
	if (isClicked)
	{
		std::vector<std::string> filePaths;
		_fileExplorer.open_file_explorer(filePaths);
		for (auto& path : filePaths)
		{
			io::URI uri = path;
			io::Utils::replace_back_slash_to_forward(uri);
			acore::ResourceImportEvent event(uri, _currentPath);
			_eventManager->trigger_event(event);
		}
	}
	ImGui::SameLine();
	isClicked = ImGui::Button("Save All");
	if (isClicked)
	{
		acore::ProjectSavingStartEvent event;
		_eventManager->trigger_event(event);
	}
	ImGui::PopStyleColor(3);
	
	for (uint32_t i = 0; i != _allPathDirectories.size(); ++i)
	{
		auto& path = _allPathDirectories[i];
		ImGui::SameLine();
		isClicked = ImGui::Button(path.c_str());
		if (isClicked)
		{
			io::URI newPath = _baseFolder;
			for (uint32_t j = 0; j != i + 1; ++j)
				newPath += "/" + _allPathDirectories[j];
			_currentPath = newPath;
			break;
		}
		if (i < _allPathDirectories.size() - 1)
		{
			ImGui::SameLine();
			ImGui::ArrowButton(("arrow" + std::to_string(i)).c_str(), ImGuiDir_Right);
		}
	}
	
	build_directories_chain();
	
	for (auto dirEntity : std::filesystem::directory_iterator(_currentPath.c_str()))
	{
		std::string fileName = io::Utils::get_file_name(dirEntity.path().string());
		if (dirEntity.is_directory())
			add_thumbnail(fileName, _icons[IconType::FOLDER]);
		else
		{
			io::URI relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), dirEntity.path().string());
			io::Utils::replace_back_slash_to_forward(relativePath);
			auto it = _resourceInfoByRelativePath.find(relativePath.c_str());
			if (it == _resourceInfoByRelativePath.end())
				continue;
			switch (it->second.type)
			{
				case resource::ResourceType::TEXTURE:
					add_thumbnail(fileName, _icons[IconType::TEXTURE_FILE]);
					break;
				case resource::ResourceType::MODEL:
					add_thumbnail(fileName, _icons[IconType::MODEL_FILE]);
					break;
				case resource::ResourceType::LEVEL:
					add_thumbnail(fileName, _icons[IconType::LEVEL_FILE]);
					break;
				case resource::ResourceType::MATERIAL:
					add_thumbnail(fileName, _icons[IconType::MATERIAL_FILE]);
					break;
			}
		}
	}
	
	_thumbnailTableManager->draw();

	if (!_thumbnailTableManager->are_any_widgets_hovered())
	{
		if (ImGui::BeginPopupContextWindow(0))
		{
			if (ImGui::MenuItem("Create new material"))
			{
				MaterialCreationWindowDrawEvent event(_currentPath);
				_eventManager->trigger_event(event);
			}
			ImGui::EndPopup();
		}
	}
		
	_thumbnailTableManager->reset();
	
	ImGui::End();
}

void ContentBrowserWindow::add_thumbnail(const std::string& label, uicore::TextureInfo textureInfo)
{
	textureInfo.width = _thumbnailSize;
	textureInfo.height = _thumbnailSize;
	_thumbnailTableManager->add_button(label, textureInfo);
}

void ContentBrowserWindow::build_directories_chain()
{
	io::URI relativePath = io::Utils::get_relative_path_to_file(_baseFolder, _currentPath);
	io::Utils::replace_back_slash_to_forward(relativePath);
	_allPathDirectories.clear();
	std::string strPath = relativePath.string();
	while (true)
	{
		if (strPath.empty()) break;
				
		uint32_t index = strPath.find("/");
		std::string directory = strPath;
		if (index <= strPath.size())
		{
			directory.erase(index, directory.size());
			strPath.erase(0, index + 1);
		}
		else
		{
			strPath.clear();
		}
		_allPathDirectories.push_back(directory);
	}
}
