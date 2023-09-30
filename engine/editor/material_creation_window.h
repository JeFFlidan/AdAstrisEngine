#pragma once

#include "common.h"
#include "ui_core/text_input_widget.h"

namespace ad_astris::editor::impl
{
	class MaterialCreationWindow : public UIWindowInternal
	{
		public:
			MaterialCreationWindow(UIWindowInitContext& initContext);
			void draw();
			void set_current_directory(io::URI path) { _currentDirectory = path; }
			bool need_drawing() { return _needDrawing; }
			void set_drawing_state(bool state) { _needDrawing = state; } 

		private:
			ResourceDesc _albedoDesc;
			ResourceDesc _metallicDesc;
			ResourceDesc _aoDesc;
			ResourceDesc _roughnessDesc;
			ResourceDesc _normalDesc;
			io::URI _currentDirectory;
			uicore::TextInputWidget _textInputWidget;
			bool _needDrawing{ false };
	};
}
