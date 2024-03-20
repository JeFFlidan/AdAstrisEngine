#include "editor.h"

#include "viewport_window.h"
#include "content_browser_window.h"
#include "properties_window.h"
#include "outliner_window.h"
#include "ui_core/utils.h"
#include "ui_core/docking_window.h"
#include "resource_manager/resource_events.h"
#include "ui_core/file_explorer.h"
#include <imgui/ImGuizmo.h>
#include <iostream>

using namespace ad_astris;
using namespace editor;
using namespace impl;

void Editor::init(EditorInitContext& initContext)
{
	assert(initContext.mainWindow != nullptr);
	assert(initContext.callbacks != nullptr);
	assert(initContext.ecsUiManager != nullptr);
	
	_fileSystem = FILE_SYSTEM();
	_eventManager = EVENT_MANAGER();
	_ecsUiManager = initContext.ecsUiManager;
	_ecsUiManager->set_global_variables();
	subscribe_to_events();

	io::URI path = _fileSystem->get_project_root_path() + "/configs/resource_table.ini";
	Config config;
	config.load_from_file(path);

	for (auto section : config)
	{
		ResourceInfo resourceInfo;
		resourceInfo.type = resource::Utils::get_enum_resource_type(section.get_option_value<std::string>("Type"));
		resourceInfo.uuid = section.get_option_value<uint64_t>("UUID");
		_resourceInfoByRelativePath[section.get_name()] = resourceInfo;
		ResourceDesc resourceDesc;
		resourceDesc.uuid = resourceInfo.uuid;
		resourceDesc.resourceName = io::Utils::get_file_name(section.get_name());
		_resourceDescriptionsByType[resourceInfo.type].push_back(resourceDesc);
	}

	rhi::UIWindowBackendCallbacks* callbacks = initContext.callbacks;
	ImGui::SetCurrentContext(callbacks->getContextCallback());
	_ecsUiManager->set_imgui_context(callbacks->getContextCallback());

	rhi::UIWindowBackendCallbacks::ImGuiAllocators imGuiAllocators = callbacks->getImGuiAllocators();
	ImGui::SetAllocatorFunctions(imGuiAllocators.allocFunc, imGuiAllocators.freeFunc);

	_getDefaultFontSize14 = initContext.callbacks->getDefaultFont14;
	_getDefaultFontSize17 = initContext.callbacks->getDefaultFont17;

	ImGui::LoadIniSettingsFromDisk((_fileSystem->get_engine_root_path() + "/configs/ui/editor.ini").c_str());
	
	_uiBeginFrameCallback = callbacks->beginFrameCallback;

	UIWindowInitContext uiWindowInitContext;
	uiWindowInitContext.eventManager = _eventManager;
	uiWindowInitContext.fileSystem = _fileSystem;
	uiWindowInitContext.ecsUiManager = _ecsUiManager;
	uiWindowInitContext.resourceInfoByRelativePath = &_resourceInfoByRelativePath;
	uiWindowInitContext.resourceNamesBytType = &_resourceDescriptionsByType;
	
	auto dockingWindow = std::make_shared<uicore::DockingWindow>();
	dockingWindow->set_window_size(initContext.mainWindow->get_width(), initContext.mainWindow->get_height());
	_viewportWindow = std::make_unique<ViewportWindow>(uiWindowInitContext,callbacks->getViewportImageCallback);
	_contentBrowserWindow = std::make_unique<ContentBrowserWindow>(uiWindowInitContext, callbacks->getContentIcons);
	_propertiesWindow = std::make_unique<PropertiesWindow>(uiWindowInitContext);
	_outlinerWindow = std::make_unique<OutlinerWindow>(uiWindowInitContext);
	_materialCreationWindow = std::make_unique<MaterialCreationWindow>(uiWindowInitContext);
	_modelCreationWindow = std::make_unique<ModelCreationWindow>(uiWindowInitContext);
	
	uicore::Utils::setup_dark_theme();
}

void Editor::cleanup()
{
	ImGui::SaveIniSettingsToDisk((_fileSystem->get_engine_root_path() + "/configs/ui/editor.ini").c_str());
}

void Editor::draw()
{
	_uiBeginFrameCallback();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	ImGui::PushFont(_getDefaultFontSize17());
	_dockingWindow.draw_window();
	_contentBrowserWindow->draw_window();
	_outlinerWindow->draw_window();
	_propertiesWindow->set_selected_entity(_outlinerWindow->get_last_selected_entity());
	_propertiesWindow->draw_window();
	_viewportWindow->set_selected_entity(_outlinerWindow->get_last_selected_entity());
	_viewportWindow->draw_window();
	if (_materialCreationWindow->need_drawing())
		_materialCreationWindow->draw();
	if (_modelCreationWindow->need_drawing())
		_modelCreationWindow->draw();
	ImGui::PopFont();
	ImGui::Render();
}

void Editor::subscribe_to_events()
{
	events::EventDelegate<resource::Texture2DCreatedEvent> delegate1 = [&](resource::Texture2DCreatedEvent& event)
	{
		ecore::Texture2D* texture = event.get_texture_handle().get_resource();
		assert(texture != nullptr);
		ResourceInfo resourceInfo;
		resourceInfo.type = resource::ResourceType::TEXTURE;
		resourceInfo.uuid = texture->get_uuid();
		io::URI relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), texture->get_path());
		io::Utils::replace_back_slash_to_forward(relativePath);
		std::scoped_lock<std::mutex> locker(_resourceInfoMutex);
		_resourceInfoByRelativePath[relativePath.c_str()] = resourceInfo;
		ResourceDesc resourceDesc;
		resourceDesc.uuid = resourceInfo.uuid;
		resourceDesc.resourceName = io::Utils::get_file_name(relativePath);
		_resourceDescriptionsByType[resourceInfo.type].push_back(resourceDesc);
	};
	_eventManager->subscribe(delegate1);
	
	events::EventDelegate<resource::StaticModelFirstCreationEvent> delegate2 = [&](resource::StaticModelFirstCreationEvent& event)
	{
		ecore::StaticModel* model = event.get_model_handle().get_resource();
		assert(model != nullptr);
		ResourceInfo resourceInfo;
		resourceInfo.type = resource::ResourceType::MODEL;
		resourceInfo.uuid = model->get_uuid();
		io::URI relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), model->get_path());
		io::Utils::replace_back_slash_to_forward(relativePath);
		std::scoped_lock<std::mutex> locker(_resourceInfoMutex);
		_resourceInfoByRelativePath[relativePath.c_str()] = resourceInfo;
		ResourceDesc resourceDesc;
		resourceDesc.uuid = resourceInfo.uuid;
		resourceDesc.resourceName = io::Utils::get_file_name(relativePath);
		_resourceDescriptionsByType[resourceInfo.type].push_back(resourceDesc);
	};
	_eventManager->subscribe(delegate2);

	events::EventDelegate<MaterialCreationWindowDrawEvent> delegate3 = [&](MaterialCreationWindowDrawEvent& event)
	{
		_materialCreationWindow->set_current_directory(event.get_current_directory());
		_materialCreationWindow->set_drawing_state(true);
	};
	_eventManager->subscribe(delegate3);

	events::EventDelegate<resource::OpaquePBRMaterialCreatedEvent> delegate4 = [&](resource::OpaquePBRMaterialCreatedEvent& event)
	{
		ecore::OpaquePBRMaterial* material = event.get_material_handle().get_resource();
		assert(material != nullptr);
		ResourceInfo resourceInfo;
		resourceInfo.type = resource::ResourceType::MATERIAL;
		resourceInfo.uuid = material->get_uuid();
		io::URI relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), material->get_path());
		io::Utils::replace_back_slash_to_forward(relativePath);
		std::scoped_lock<std::mutex> locker(_resourceInfoMutex);
		_resourceInfoByRelativePath[relativePath.c_str()] = resourceInfo;
		ResourceDesc resourceDesc;
		resourceDesc.uuid = resourceInfo.uuid;
		resourceDesc.resourceName = io::Utils::get_file_name(relativePath);
		_resourceDescriptionsByType[resourceInfo.type].push_back(resourceDesc);
	};
	_eventManager->subscribe(delegate4);

	events::EventDelegate<ModelCreationWindowDrawEvent> delegate5 = [&](ModelCreationWindowDrawEvent& event)
	{
		_modelCreationWindow->set_drawing_state(true);
	};
	_eventManager->subscribe(delegate5);
}
