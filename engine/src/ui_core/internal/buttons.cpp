#include "buttons.h"

using namespace ad_astris::uicore::internal;

Button::Button(const std::string& buttonName, const ImVec2& buttonSize) : BaseWidget(buttonName), _buttonSize(buttonSize)
{
	
}

bool Button::draw()
{
	return ImGui::Button(_name.c_str(), _buttonSize);
}

ImageButton::ImageButton(const std::string& buttonName, TextureInfo& textureInfo) : BaseWidget(buttonName), _textureInfo(textureInfo)
{
	
}

bool ImageButton::draw()
{
	ImVec2 size( _textureInfo.width, _textureInfo.height );
	return ImGui::ImageButton(_name.c_str(), (void*)(intptr_t)_textureInfo.textureID, size, _textureInfo.uv0, _textureInfo.uv1);
}
