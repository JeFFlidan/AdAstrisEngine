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

	uicore::ComboBoxValueGetter<ResourceDesc> callback = [&](const ResourceDesc& desc)->const char* { return desc.resourceName.c_str(); };

	_albedoTexturesComboBox = std::make_unique<ResourceComboBox>("Albedo", textureDescs, callback);
	_albedoTexturesComboBox->set_default_object(textureDescs[0]);
	_normalTexturesComboBox = std::make_unique<ResourceComboBox>("Normal", textureDescs, callback);
	_normalTexturesComboBox->set_default_object(textureDescs[0]);
	_roughnessTexturesComboBox = std::make_unique<ResourceComboBox>("Roughness", textureDescs, callback);
	_roughnessTexturesComboBox->set_default_object(textureDescs[0]);
	_metallicTexturesComboBox = std::make_unique<ResourceComboBox>("Metallic", textureDescs, callback);
	_metallicTexturesComboBox->set_default_object(textureDescs[0]);
	_aoTexturesComboBox = std::make_unique<ResourceComboBox>("Ambient Occlusion", textureDescs, callback);
	_aoTexturesComboBox->set_default_object(textureDescs[0]);
}

void MaterialCreationWindow::draw()
{
	ImGui::Begin("Create Material");
	_textInputWidget.draw();
	
	_albedoTexturesComboBox->draw();
	_normalTexturesComboBox->draw();
	_roughnessTexturesComboBox->draw();
	_metallicTexturesComboBox->draw();
	_aoTexturesComboBox->draw();

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
			_albedoTexturesComboBox->get_selected_object().uuid,
			_normalTexturesComboBox->get_selected_object().uuid,
			_roughnessTexturesComboBox->get_selected_object().uuid,
			_metallicTexturesComboBox->get_selected_object().uuid,
			_aoTexturesComboBox->get_selected_object().uuid);
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
