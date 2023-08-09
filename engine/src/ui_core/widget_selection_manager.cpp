#include "widget_selection_manager.h"
#include "internal/buttons.h"
#include "utils.h"
#include "profiler/logger.h"
#include <imgui.h>

using namespace ad_astris::uicore;

WidgetSelectionManager::WidgetSelectionManager()
{
	create_default_style_callback();
}

WidgetSelectionManager::WidgetSelectionManager(WidgetSelectionManagerCreationContext& creationContext)
{
	_multipleSelection = creationContext.multipleSelection;
	if (creationContext.setCustomStyleCallback == nullptr)
	{
		create_default_style_callback();
	}
	else
	{
		_setCustomStyleCallback = creationContext.setCustomStyleCallback;
		_countOfColorStylePushes = creationContext.countOfColorStylePushes;
		_countOfStyleVarPushes = creationContext.countOfStyleVarPushes;
	}
	_hoveredWidgetMouseDoubleClickedCallback = creationContext.hoveredWidgetMouseDoubleClickedCallback;
	_hoveredWidgetRightMouseButtonClickedCallback = creationContext.hoveredWidgetRightMouseButtonClickedCallback;
	_drawImageButtonLabel = creationContext.drawImageButtonLabel;
	_editableButtonLabel = creationContext.editableButtonLabel;
	_drawButtonsHorizontally = creationContext.drawButtonsHorizontally;
	_permanentSelection = creationContext.permanentSelection;
}

void WidgetSelectionManager::add_button(const std::string& buttonLabel, const ImVec2& buttonSize, bool isSelectedInitially)
{
	auto button = std::make_unique<internal::Button>(buttonLabel, buttonSize);
	_widgets.emplace_back(std::move(button));
	if (isSelectedInitially)
		_selectedWidgetNames.insert(buttonLabel);
}

void WidgetSelectionManager::add_image_button(const std::string& buttonName, TextureInfo& textureInfo, bool isSelectedInitially)
{
	auto button = std::make_unique<internal::ImageButton>(buttonName, textureInfo); 
	_widgets.emplace_back(std::move(button));
	if (isSelectedInitially)
		_selectedWidgetNames.insert(buttonName);
}

std::unordered_set<std::string>& WidgetSelectionManager::draw_widgets()
{
	for (uint32_t i = 0; i != _widgets.size(); ++i)
		draw_one_widget(i);

	return _selectedWidgetNames;
}

bool WidgetSelectionManager::draw_one_widget(uint32_t widgetIndex)
{
	internal::BaseWidget* baseWidget = _widgets[widgetIndex].get();
	
	if (_drawButtonsHorizontally)
		ImGui::SameLine();
	if (_selectedWidgetNames.find(baseWidget->get_name()) != _selectedWidgetNames.end())
	{
		_setCustomStyleCallback();
	}
	bool widgetPressed = baseWidget->draw();
	if (_drawImageButtonLabel)
		ImGui::Text(baseWidget->get_name().c_str());
	if (_selectedWidgetNames.find(baseWidget->get_name()) != _selectedWidgetNames.end())
	{
		ImGui::PopStyleColor(_countOfColorStylePushes);
		ImGui::PopStyleVar(_countOfStyleVarPushes);
	}

	bool noSelectionButtonsPressed = (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !(ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl));\
	if (!_permanentSelection && !widgetPressed && noSelectionButtonsPressed && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		_selectedWidgetNames.clear();
		_selectedWidgetIndices.clear();
	}

	if (widgetPressed)
	{
		if (!_multipleSelection)
		{
			_selectedWidgetNames.clear();
			_selectedWidgetNames.insert(baseWidget->get_name());
		}
		else
		{
			bool wasButtonSelected = _selectedWidgetNames.find(baseWidget->get_name()) != _selectedWidgetNames.end();
			if (!wasButtonSelected && (ImGui::IsKeyDown(ImGuiKey_LeftShift) || (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))))
			{
				if (_selectedWidgetNames.empty())
				{
					_selectedWidgetNames.insert(baseWidget->get_name());
					_selectedWidgetIndices.push_back(widgetIndex);
				}
				else
				{
					uint32_t lastSelectedWidget = _selectedWidgetIndices.back();
					_selectedWidgetIndices.push_back(widgetIndex);
					_selectedWidgetNames.insert(baseWidget->get_name());

					uint32_t beginningIndex, endIndex;
					if (lastSelectedWidget < widgetIndex)
					{
						beginningIndex = lastSelectedWidget + 1;
						endIndex = widgetIndex;
					}
					else
					{
						beginningIndex = widgetIndex;
						endIndex = lastSelectedWidget - 1;
					}
					
					for (; beginningIndex <= endIndex; ++beginningIndex)
					{
						_selectedWidgetIndices.push_back(beginningIndex);
						_selectedWidgetNames.insert(_widgets[beginningIndex]->get_name());
					}
				}
			}
			else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				_selectedWidgetIndices.push_back(widgetIndex);
				_selectedWidgetNames.insert(baseWidget->get_name());
			}
			else
			{
				_selectedWidgetIndices.clear();
				_selectedWidgetNames.clear();
				_selectedWidgetIndices.push_back(widgetIndex);
				_selectedWidgetNames.insert(baseWidget->get_name());
			}
		}
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		if (_hoveredWidgetMouseDoubleClickedCallback)
			_hoveredWidgetMouseDoubleClickedCallback(*baseWidget);
	}
	if (ImGui::IsItemHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		if (_hoveredWidgetRightMouseButtonClickedCallback)
			_hoveredWidgetRightMouseButtonClickedCallback(*baseWidget);
	}
	
	return widgetPressed;
}

void WidgetSelectionManager::create_default_style_callback()
{
	_setCustomStyleCallback = []()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
		ImGui::PushStyleColor(ImGuiCol_Button, Utils::get_default_fill_color());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Utils::get_default_hover_color());
	};
	_countOfColorStylePushes = 2;
	_countOfStyleVarPushes = 1;
}
