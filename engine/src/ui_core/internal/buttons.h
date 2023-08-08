#pragma once

#include "base_widget.h"
#include "ui_core/common.h"
#include <imgui.h>

namespace ad_astris::uicore::internal
{
	class Button : public BaseWidget
	{
		public:
			Button(const std::string& buttonName, const ImVec2& buttonSize = { 0, 0 });
			virtual bool draw() override;

		private:
			ImVec2 _buttonSize;
	};

	class ImageButton : public BaseWidget
	{
		public:
			ImageButton(const std::string& buttonName, TextureInfo& textureInfo);
			virtual bool draw() override;

		private:
			TextureInfo _textureInfo;
	};
}
