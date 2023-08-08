#include "last_selection_highlight_manager.h"
#include "internal/buttons.h"
#include "utils.h"
#include <imgui.h>

using namespace ad_astris::uicore;

LastSelectionHighlightManager::LastSelectionHighlightManager()
{
	_funcToSetupColorStyle = []()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
		ImGui::PushStyleColor(ImGuiCol_Button, Utils::get_normalized_color(239.0f, 151.0f, 133.0f, 0.75f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Utils::get_normalized_color(255.0f, 118.0f, 40.0f, 1.0f));
	};
	_countOfColorStylePushes = 2;
	_countOfStyleVarPushes = 1;
}

LastSelectionHighlightManager::LastSelectionHighlightManager(
	std::function<void()>& funcToSetupColorStyle,
	uint32_t countOfColorStylePushes,
	uint32_t countOfStyleVarPushes)
		: _funcToSetupColorStyle(funcToSetupColorStyle), _countOfColorStylePushes(countOfColorStylePushes), _countOfStyleVarPushes(countOfStyleVarPushes)
{
	
}

void LastSelectionHighlightManager::add_button(const std::string& buttonLabel, const ImVec2& buttonSize, bool isSelectedInitially)
{
	auto button = std::make_unique<internal::Button>(buttonLabel, buttonSize);
	_widgets.emplace_back(std::move(button));
	if (isSelectedInitially)
		_currentSelectedButton = buttonLabel;
}

void LastSelectionHighlightManager::add_image_button(const std::string& buttonName, TextureInfo& textureInfo, bool isSelectedInitially)
{
	auto button = std::make_unique<internal::ImageButton>(buttonName, textureInfo); 
	_widgets.emplace_back(std::move(button));
	if (isSelectedInitially)
		_currentSelectedButton = buttonName;
}

std::string LastSelectionHighlightManager::draw_buttons(bool drawButtonsHorizontally)
{
	std::string pressedButtonName{};
	
	for (auto& button : _widgets)
	{
		if (drawButtonsHorizontally)
			ImGui::SameLine();
		if (button->get_name() == _currentSelectedButton)
			_funcToSetupColorStyle();
		bool buttonPressed = button->draw();
		if (button->get_name() == _currentSelectedButton)
		{
			ImGui::PopStyleColor(_countOfColorStylePushes);
			ImGui::PopStyleVar(_countOfStyleVarPushes);
		}

		if (buttonPressed)
		{
			pressedButtonName = button->get_name();
			_currentSelectedButton = button->get_name();
		}
	}

	return pressedButtonName;
}
