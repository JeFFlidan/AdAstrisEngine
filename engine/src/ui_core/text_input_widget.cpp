#include "text_input_widget.h"

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace ad_astris::uicore;

TextInputWidget::TextInputWidget(const std::string& label, float inputTextFieldWidth, const std::string& defaultText, bool editable, uint32_t bufferSize)
	: BaseWidget(label), _text(defaultText), _editable(editable), _bufferSize(bufferSize), _inputTextFieldWidth(inputTextFieldWidth)
{
	_text.reserve(_bufferSize);
}

bool TextInputWidget::draw()
{
	if (_editable)
	{
		ImGui::PushItemWidth(_inputTextFieldWidth);
		bool result = ImGui::InputText(_name.c_str(), &_text);
		ImGui::PopItemWidth();
		return result;
	}

	ImGui::PushItemWidth(_inputTextFieldWidth);
	ImGui::InputText(_name.c_str(), &_text, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();
	return false;
}
