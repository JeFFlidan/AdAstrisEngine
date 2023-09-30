#include "outliner_window.h"
#include "application_core/editor_events.h"
#include "engine_core/basic_components.h"
#include "engine_core/basic_events.h"
#include "ui_core/font_manager.h"
#include "ui_core/utils.h"
#include <imgui/imgui.h>

using namespace ad_astris;
using namespace editor;
using namespace impl;

OutlinerWindow::OutlinerWindow(UIWindowInitContext& initContext) : UIWindowInternal(initContext)
{
	subscribe_to_events();
}

void OutlinerWindow::draw_window(void* data)
{
	ImGui::Begin("Outliner");
	for (auto& entityName : _entityNames)
		draw_entity_node(entityName);
	
	if (ImGui::BeginPopupContextWindow(0))
	{
		if (ImGui::MenuItem("Create new point light"))
		{
			acore::EditorPointLightCreationEvent event;
			_eventManager->trigger_event(event);
		}
		if (ImGui::MenuItem("Create new static model"))
		{
			ModelCreationWindowDrawEvent event;
			_eventManager->trigger_event(event);
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void OutlinerWindow::subscribe_to_events()
{
	events::EventDelegate<ecore::EntityCreatedEvent> delegate1 = [&](ecore::EntityCreatedEvent& event)
	{
		generate_entity_name(event.get_entity());
	};
	_eventManager->subscribe(delegate1);
}

void OutlinerWindow::generate_entity_name(ecs::Entity entity)
{
	auto addNameFunc = [&](const std::string& name)
	{
		auto it = _nameCounterByDefaultName.find(name);
		if (it == _nameCounterByDefaultName.end())
		{
			_nameCounterByDefaultName[name] = 1;
			_entityByItsName[name] = entity;
			_entityNames.push_back(name);
			_lastSelectedEntityName = name;
			std::sort(_entityNames.begin(), _entityNames.end());
		}
		else
		{
			_entityByItsName[name + std::to_string(it->second)] = entity;
			_entityNames.push_back(name + std::to_string(it->second++));
			//_lastSelectedEntityName = _entityNames.back();
			std::sort(_entityNames.begin(), _entityNames.end());
		}
	};
	
	if (_ecsUiManager->does_entity_have_tag<ecore::DirectionalLightTag>(entity))
	{
		addNameFunc("DirectionalLight");
	}
	else if (_ecsUiManager->does_entity_have_tag<ecore::SpotLightTag>(entity))
	{
		addNameFunc("SpotLight");
	}
	else if (_ecsUiManager->does_entity_have_tag<ecore::PointLightTag>(entity))
	{
		addNameFunc("PointLight");
	}
	else if (_ecsUiManager->does_entity_have_tag<ecore::StaticObjectTag>(entity) && _ecsUiManager->does_entity_have_component<ecore::ModelComponent>(entity))
	{
		addNameFunc("StaticModel");
	}
}

void OutlinerWindow::draw_entity_node(std::string& entityName)
{
	ecs::Entity entity = _entityByItsName[entityName];
	ImGuiTreeNodeFlags flags = (entityName == _lastSelectedEntityName ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;

	if (entityName == _lastSelectedEntityName)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, uicore::Utils::get_default_fill_color());
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, uicore::Utils::get_default_hover_color());
	}
	
	bool isOpened = ImGui::TreeNodeEx((void*)(uint64_t)entity.get_uuid(), flags, entityName.c_str());

	if (entityName == _lastSelectedEntityName)
		ImGui::PopStyleColor(2);
	
	if (ImGui::IsItemClicked())
		_lastSelectedEntityName = entityName;
	if (isOpened)
		ImGui::TreePop();
	if (ImGui::IsItemHovered())
		_areAnyEntitiesSelected = true;
}
