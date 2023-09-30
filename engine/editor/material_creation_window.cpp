#include "material_creation_window.h"
#include "application_core/editor_events.h"
#include "ui_core/utils.h"
#include "ui_core/text_input_widget.h"

using namespace ad_astris;
using namespace editor;
using namespace impl;

MaterialCreationWindow::MaterialCreationWindow(UIWindowInitContext& initContext) : UIWindowInternal(initContext)
{
	_textInputWidget = uicore::TextInputWidget("Material name", 256, "OpaquePBRMaterial");
	std::vector<ResourceDesc>& textureDescs = _resourceDescriptionsByType[resource::ResourceType::TEXTURE];
	_albedoDesc = textureDescs[0];
	_metallicDesc = textureDescs[0];
	_aoDesc = textureDescs[0];
	_normalDesc = textureDescs[0];
	_roughnessDesc = textureDescs[0];
}

void MaterialCreationWindow::draw()
{
	ImGui::Begin("Create Material");
	_textInputWidget.draw();

	std::vector<ResourceDesc>& textureDescs = _resourceDescriptionsByType[resource::ResourceType::TEXTURE];

	if (ImGui::BeginCombo("Albedo", _albedoDesc.resourceName.c_str()))
	{
		for (auto& textureDesc : textureDescs)
		{
			bool isSelected = _albedoDesc.resourceName == textureDesc.resourceName;
			if (ImGui::Selectable(textureDesc.resourceName.c_str(), isSelected))
				_albedoDesc = textureDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Normal", _normalDesc.resourceName.c_str()))
	{
		for (auto& textureDesc : textureDescs)
		{
			bool isSelected = _normalDesc.resourceName == textureDesc.resourceName;
			if (ImGui::Selectable(textureDesc.resourceName.c_str(), isSelected))
				_normalDesc = textureDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Roughness", _roughnessDesc.resourceName.c_str()))
	{
		for (auto& textureDesc : textureDescs)
		{
			bool isSelected = _roughnessDesc.resourceName == textureDesc.resourceName;
			if (ImGui::Selectable(textureDesc.resourceName.c_str(), isSelected))
				_roughnessDesc = textureDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Metallic", _metallicDesc.resourceName.c_str()))
	{
		for (auto& textureDesc : textureDescs)
		{
			bool isSelected = _metallicDesc.resourceName == textureDesc.resourceName;
			if (ImGui::Selectable(textureDesc.resourceName.c_str(), isSelected))
				_metallicDesc = textureDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Ambient Occlusion", _aoDesc.resourceName.c_str()))
	{
		for (auto& textureDesc : textureDescs)
        {
        	bool isSelected = _aoDesc.resourceName == textureDesc.resourceName;
        	if (ImGui::Selectable(textureDesc.resourceName.c_str(), isSelected))
        		_aoDesc = textureDesc;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
        }
		ImGui::EndCombo();
	}

	ImVec2 regionAvail = ImGui::GetContentRegionAvail();
	ImGui::SetCursorPos(ImVec2(regionAvail.x - 150, regionAvail.y + 150));
	
	ImGui::PushStyleColor(ImGuiCol_Button, uicore::Utils::get_default_fill_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uicore::Utils::get_default_hover_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, uicore::Utils::get_default_hover_color());
	if (ImGui::Button("Create"))
	{
		acore::OpaquePBRMaterialCreationEvent event(
			_currentDirectory,
			_textInputWidget.get_text(),
			_albedoDesc.uuid,
			_normalDesc.uuid,
			_roughnessDesc.uuid,
			_metallicDesc.uuid,
			_aoDesc.uuid);
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
