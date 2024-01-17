#include "model_creation_window.h"
#include "ui_core/utils.h"
#include "application_core/editor_events.h"

using namespace ad_astris;
using namespace editor;
using namespace impl;

ModelCreationWindow::ModelCreationWindow(UIWindowInitContext& initContext) : UIWindowInternal(initContext)
{
	std::vector<ResourceDesc>& modelDescs = _resourceDescriptionsByType[resource::ResourceType::MODEL];
	std::vector<ResourceDesc>& materialDescs = _resourceDescriptionsByType[resource::ResourceType::MATERIAL];

	_modelDesc = modelDescs[0];
	_materialDesc = materialDescs[0];
}

void ModelCreationWindow::draw()
{
	ImGui::Begin("Create Static Model");

	std::vector<ResourceDesc>& modelDescs = _resourceDescriptionsByType[resource::ResourceType::MODEL];
	std::vector<ResourceDesc>& materialDescs = _resourceDescriptionsByType[resource::ResourceType::MATERIAL];

	if (ImGui::BeginCombo("Model", _modelDesc.resourceName.c_str()))
	{
		for (auto& modelDesc : modelDescs)
		{
			bool isSelected = _modelDesc.resourceName == modelDesc.resourceName;
			if (ImGui::Selectable(modelDesc.resourceName.c_str(), isSelected))
				_modelDesc = modelDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Material", _materialDesc.resourceName.c_str()))
	{
		for (auto& materialDesc : materialDescs)
		{
			bool isSelected = _materialDesc.resourceName == materialDesc.resourceName;
			if (ImGui::Selectable(materialDesc.resourceName.c_str(), isSelected))
				_materialDesc = materialDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImVec2 regionAvail = ImGui::GetContentRegionAvail();
	ImGui::SetCursorPos(ImVec2(regionAvail.x - 150, regionAvail.y + 50));

	ImGui::PushStyleColor(ImGuiCol_Button, uicore::Utils::get_default_fill_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uicore::Utils::get_default_hover_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, uicore::Utils::get_default_hover_color());
	if (ImGui::Button("Create"))
	{
		acore::EditorStaticModelCreationEvent event(_modelDesc.uuid, _materialDesc.uuid);
		_eventManager->trigger_event(event);
		_needDrawing = false;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		_needDrawing = false;
	}
	
	ImGui::End();
}
