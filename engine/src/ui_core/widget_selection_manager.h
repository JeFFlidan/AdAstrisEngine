#pragma once

#include "internal/base_widget.h"
#include "common.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include <memory>

namespace ad_astris::uicore
{
	struct WidgetSelectionManagerCreationContext
	{
		bool multipleSelection;
		UISetCustomStyleCallback setCustomStyleCallback{ nullptr };
		uint32_t countOfColorStylePushes{ 0 };
		uint32_t countOfStyleVarPushes{ 0 };
		UICallback hoveredWidgetMouseDoubleClickedCallback{ nullptr };
		UICallback hoveredWidgetRightMouseButtonClickedCallback{ nullptr };
		bool drawImageButtonLabel{ false };
		bool editableButtonLabel{ false };
		bool drawButtonsHorizontally{ false };
		bool permanentSelection{ false };
	};
	
	/** This class drawing some previously added widgets and highlights
	selected widgets. It works for one selected widget and for
	several selected widgets. Default color style is for buttons,
	however you can change it using the constructor.
	Default highlight color is: 39.0f, 151.0f, 133.0f, 0.75f. 
	Default hover color is: 255.0f, 118.0f, 40.0f, 1.0f. */
	class WidgetSelectionManager
	{
		public:
			// Setup default ui style
			WidgetSelectionManager();
			// Setup custom ui style
			WidgetSelectionManager(WidgetSelectionManagerCreationContext& creationContext);

			void add_button(const std::string& buttonLabel, const ImVec2& buttonSize = { 0, 0 }, bool isSelectedInitially = false);
			void add_image_button(const std::string& buttonName, TextureInfo& textureInfo, bool isSelectedInitially = false);
			/**
			 * 
			 * \return Returns the name of the pressed button. If no button is pressed, returns an empty string
			 */
			std::unordered_set<std::string>& draw_widgets();
			bool draw_one_widget(uint32_t widgetIndex);

			uint32_t get_widgets_count()
			{
				return _widgets.size();
			}
		
			std::unordered_set<std::string>& get_current_selected_widget_names()
			{
				return _selectedWidgetNames;
			}

			void reset();
			bool are_any_widgets_hovered() { return _areAnyWidgetsHovered; }

		private:
			std::vector<std::unique_ptr<internal::BaseWidget>> _widgets;
			std::vector<uint32_t> _selectedWidgetIndices;
			std::unordered_set<std::string> _selectedWidgetNames;
			UISetCustomStyleCallback _setCustomStyleCallback{ nullptr };
			uint32_t _countOfColorStylePushes;
			uint32_t _countOfStyleVarPushes;
			UICallback _hoveredWidgetMouseDoubleClickedCallback{ nullptr };
			UICallback _hoveredWidgetRightMouseButtonClickedCallback{ nullptr };
			bool _multipleSelection;
			bool _drawImageButtonLabel{ false };
			bool _editableButtonLabel{ false };
			bool _drawButtonsHorizontally{ false };
			bool _permanentSelection{ false };
			bool _areAnyWidgetsHovered{ false };

			void create_default_style_callback();
	};
}
