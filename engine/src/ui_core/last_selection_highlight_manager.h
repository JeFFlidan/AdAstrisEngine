#pragma once

#include "internal/base_widget.h"
#include "common.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace ad_astris::uicore
{
	/** This class drawing some previously added widgets and highlights
	the last selected one until the next selection. Default color style
	is for buttons, however you can change it using the constructor.
	Default highlight color is: 39.0f, 151.0f, 133.0f, 0.75f. 
	Default hover color is: 255.0f, 118.0f, 40.0f, 1.0f. */
	class LastSelectionHighlightManager
	{
		public:
			// Setup default ui style
			LastSelectionHighlightManager();
			// Setup custom ui style
			LastSelectionHighlightManager(
				std::function<void()>& funcToSetupColorStyle,
				uint32_t countOfColorStylePushes,
				uint32_t countOfStyleVarPushes);

			void add_button(const std::string& buttonLabel, const ImVec2& buttonSize = { 0, 0 }, bool isSelectedInitially = false);
			void add_image_button(const std::string& buttonName, TextureInfo& textureInfo, bool isSelectedInitially = false);
			/**
			 * 
			 * \return Returns the name of the pressed button. If no button is pressed, returns an empty string
			 */
			std::string draw_buttons(bool drawButtonsHorizontally = false);

		private:
			std::vector<std::unique_ptr<internal::BaseWidget>> _widgets;
			std::string _currentSelectedButton;
			std::function<void()> _funcToSetupColorStyle;
			uint32_t _countOfColorStylePushes;
			uint32_t _countOfStyleVarPushes;
	};
}
