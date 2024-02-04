#include "ecs_ui_manager.h"
#include <imgui/imgui.h>

using namespace ad_astris;
using namespace uicore;

ECSUiManager::ECSUiManager(ecs::EntityManager* entityManager)
	: _entityManager(entityManager)
{
	
}

void ECSUiManager::set_imgui_context(ImGuiContext* context)
{
	_imGuiContext = context;
}

void ECSUiManager::set_global_variables()
{
	
}

void ECSUiManager::draw_component_widgets(ecs::Entity entity)
{
	std::vector<uint64_t> componentIDs;
	_entityManager->get_entity_all_component_ids(entity, componentIDs);

	for (auto& id : componentIDs)
	{
		void* component = _entityManager->get_entity_component_by_id(entity, id);
		WidgetDesc& widgetDesc = _widgetDescByComponentID[id];
		widgetDesc.callback(component, widgetDesc.widgetProperties);
	}
}

XMFLOAT2 uicore::draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT2 vec, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	float vecArray[2] = { vec.x, vec.y };
	ImGui::DragFloat2(elementName.c_str(), vecArray, widgetProperties.speed, widgetProperties.minValueFloat, widgetProperties.maxValueFloat);
	return XMFLOAT2(vecArray[0], vecArray[1]);
}

XMFLOAT3 uicore::draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT3 vec, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	float vecArray[3] = { vec.x, vec.y, vec.z };
	ImGui::DragFloat3(elementName.c_str(), vecArray, widgetProperties.speed, widgetProperties.minValueFloat, widgetProperties.maxValueFloat);
	return XMFLOAT3(vecArray[0], vecArray[1], vecArray[2]);
}

XMFLOAT4 uicore::draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT4 vec, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	float vecArray[4] = { vec.x, vec.y, vec.z, vec.w };
	ImGui::DragFloat4(elementName.c_str(), vecArray, widgetProperties.speed, widgetProperties.minValueFloat, widgetProperties.maxValueFloat);
	return XMFLOAT4(vecArray[0], vecArray[1], vecArray[2], vecArray[3]);
}

UUID uicore::draw_element(ImGuiContext* context, const std::string& elementName, UUID uuid, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	// TODO
	return uuid;
}

float uicore::draw_element(ImGuiContext* context, const std::string& elementName, float val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	ImGui::SliderFloat(elementName.c_str(), &val, widgetProperties.minValueFloat, widgetProperties.maxValueFloat);
	return val;
}

double uicore::draw_element(ImGuiContext* context, const std::string& elementName, double val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	// TODO
	return val;
}

bool uicore::draw_element(ImGuiContext* context, const std::string& elementName, bool val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	ImGui::Checkbox(elementName.c_str(), &val);
	return val;
}

int32_t uicore::draw_element(ImGuiContext* context, const std::string& elementName, int32_t val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	ImGui::DragInt(elementName.c_str(), &val, widgetProperties.speed, widgetProperties.minValueInt, widgetProperties.maxValueInt);
	return val;
}

int64_t uicore::draw_element(ImGuiContext* context, const std::string& elementName, int64_t val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	return val;
}

uint32_t uicore::draw_element(ImGuiContext* context, const std::string& elementName, uint32_t val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	int32_t tempVal = val;
	uint32_t minValue = widgetProperties.minValueInt >= 0 ? widgetProperties.minValueInt : 0;
	ImGui::DragInt(elementName.c_str(), &tempVal, widgetProperties.speed, minValue, widgetProperties.maxValueInt);
	return tempVal;
}

uint64_t uicore::draw_element(ImGuiContext* context, const std::string& elementName, uint64_t val, ComponentWidgetProperties& widgetProperties)
{
	ImGui::SetCurrentContext(context);
	// TODO
	return val;
}
