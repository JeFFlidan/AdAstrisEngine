#include "properties_window.h"
#include "engine_core/basic_components.h"
#include "engine_core/basic_events.h"
#include "imgui/imgui.h"

using namespace ad_astris;
using namespace editor;
using namespace impl;

PropertiesWindow::PropertiesWindow(UIWindowInitContext& initContext) : UIWindowInternal(initContext)
{
	set_transform_component_widgets();
	set_color_component_widget();
	set_material_component_widget();
}

void PropertiesWindow::draw_window(void* data)
{
	ImGui::Begin("Properties");
	_ecsUiManager->draw_component_widgets(_selectedEntity);
	ImGui::End();
}

void PropertiesWindow::set_transform_component_widgets()
{
	_ecsUiManager->set_custom_callback<ecore::TransformComponent>([&](void* data, uicore::ComponentWidgetProperties& properties)
	{
		ecore::TransformComponent& component = *static_cast<ecore::TransformComponent*>(data);
		ImGui::DragFloat3("Location", &component.location.x, 0.3f, -100000.0f, 100000.0f);
		
		XMFLOAT3 prevRotationEuler = component.rotationEuler;
		bool isUsed = ImGui::DragFloat3("Rotation", &prevRotationEuler.x, 0.2f, -10000.0f, 10000.0f);

		if (isUsed)
		{
			XMVECTOR quat = XMLoadFloat4(&component.rotation);
			if (prevRotationEuler.x != component.rotationEuler.x)
			{
				XMVECTOR x = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(prevRotationEuler.x - component.rotationEuler.x), 0, 0);
				quat = XMQuaternionMultiply(x, quat);
			}
			if (prevRotationEuler.y != component.rotationEuler.y)
			{
				XMVECTOR y = XMQuaternionRotationRollPitchYaw(0, XMConvertToRadians(prevRotationEuler.y - component.rotationEuler.y), 0);
				quat = XMQuaternionMultiply(quat, y);
			}
			if (prevRotationEuler.z != component.rotationEuler.z)
			{
				XMVECTOR z = XMQuaternionRotationRollPitchYaw(0, 0, XMConvertToRadians(prevRotationEuler.z - component.rotationEuler.z));
				quat = XMQuaternionMultiply(z, quat);
			}

			quat = XMQuaternionNormalize(quat);
			XMStoreFloat4(&component.rotation, quat);
			component.rotationEuler = prevRotationEuler;
		}
		
		ImGui::DragFloat3("Scale", &component.scale.x, 0.3f, 0.0f, 100000.0f);
	});
}

void PropertiesWindow::set_color_component_widget()
{
	_ecsUiManager->set_custom_callback<ecore::ColorComponent>([&](void* data, uicore::ComponentWidgetProperties&)
	{
		ecore::ColorComponent* colorComponent = static_cast<ecore::ColorComponent*>(data);
		ImGui::ColorEdit4("Color", &colorComponent->color.x);
	});
}

void PropertiesWindow::set_material_component_widget()
{
	_ecsUiManager->set_custom_callback<ecore::OpaquePBRMaterialComponent>([&](void* data, uicore::ComponentWidgetProperties&)
	{
		auto material = static_cast<ecore::OpaquePBRMaterialComponent*>(data);
		std::vector<ResourceDesc> materialDescriptions = _resourceDescriptionsByType[resource::ResourceType::MATERIAL];
		std::string materialName;
		for (auto& desc : materialDescriptions)
		{
			if (desc.uuid == material->materialUUID)
			{
				materialName = desc.resourceName;
				break;
			}
		}

		if (ImGui::BeginCombo("Materials", materialName.c_str()))
		{
			for (auto& materialDesc : materialDescriptions)
			{
				bool isSelected = materialName == materialDesc.resourceName;
				if (ImGui::Selectable(materialDesc.resourceName.c_str(), isSelected))
				{
					material->materialUUID = materialDesc.uuid;
					LOG_INFO("MATERIAL UUID: {}", material->materialUUID)
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	});
}
