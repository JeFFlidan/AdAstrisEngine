#pragma once

#include "common.h"
#include "ui_core/text_input_widget.h"
#include "ui_core/combo_box.h"

namespace ad_astris::editor::impl
{
	using ResourceComboBox = uicore::ComboBox<ResourceDesc, std::vector>;
	
	class MaterialCreationWindow : public UIWindowInternal
	{
		public:
			MaterialCreationWindow(UIWindowInitContext& initContext);
			void draw();
			void set_current_directory(io::URI path) { _currentDirectory = path; }
			bool need_drawing() { return _needDrawing; }
			void set_drawing_state(bool state) { _needDrawing = state; } 

		private:
			std::unique_ptr<ResourceComboBox> _albedoTexturesComboBox;
			std::unique_ptr<ResourceComboBox> _metallicTexturesComboBox;
			std::unique_ptr<ResourceComboBox> _aoTexturesComboBox;
			std::unique_ptr<ResourceComboBox> _roughnessTexturesComboBox;
			std::unique_ptr<ResourceComboBox> _normalTexturesComboBox;
			io::URI _currentDirectory;
			uicore::TextInputWidget _textInputWidget;
			bool _needDrawing{ false };
	};
}
