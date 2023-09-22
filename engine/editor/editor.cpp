#include "editor.h"

#include "viewport_window.h"
#include "content_browser_window.h"
#include "properties_window.h"
#include "outliner_window.h"
#include "ui_core/utils.h"
#include "ui_core/docking_window.h"
#include "ui_core/file_explorer.h"
#include <iostream>

using namespace ad_astris;
using namespace editor;
using namespace impl;

void Editor::init(EditorInitContext& initContext)
{
	assert(initContext.eventManager != nullptr);
	assert(initContext.fileSystem != nullptr);
	assert(initContext.mainWindow != nullptr);
	assert(initContext.callbacks != nullptr);
	
	_fileSystem = initContext.fileSystem;
	_eventManager = initContext.eventManager;

	rhi::UIWindowBackendCallbacks* callbacks = initContext.callbacks;
	ImGui::SetCurrentContext(callbacks->getContextCallback());

	rhi::UIWindowBackendCallbacks::ImGuiAllocators imGuiAllocators = callbacks->getImGuiAllocators();
	ImGui::SetAllocatorFunctions(imGuiAllocators.allocFunc, imGuiAllocators.freeFunc);

	ImGui::LoadIniSettingsFromDisk((_fileSystem->get_engine_root_path() + "/configs/editor.ini").c_str());
	
	_uiBeginFrameCallback = callbacks->beginFrameCallback;
	
	auto dockingWindow = std::make_shared<uicore::DockingWindow>();
	dockingWindow->set_window_size(initContext.mainWindow->get_width(), initContext.mainWindow->get_height());
	_uiWindows.push_back(dockingWindow);
	_uiWindows.emplace_back(new ViewportWindow(callbacks->getViewportImageCallback));
	_uiWindows.emplace_back(new ContentBrowserWindow());
	_uiWindows.emplace_back(new PropertiesWindow());
	_uiWindows.emplace_back(new OutlinerWindow());

	uicore::Utils::setup_dark_theme();
}

void Editor::cleanup()
{
	ImGui::SaveIniSettingsToDisk((_fileSystem->get_engine_root_path() + "/configs/editor.ini").c_str());
}

void Editor::draw()
{
	_uiBeginFrameCallback();
	ImGui::NewFrame();
	for (auto& uiWindow : _uiWindows)
		uiWindow->draw_window();
	ImGui::Render();
}
