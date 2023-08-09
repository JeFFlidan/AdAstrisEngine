#pragma once

#include "internal/base_widget.h"

namespace ad_astris::uicore
{
	class TextInputWidget : public internal::BaseWidget
	{
		public:
			TextInputWidget() = default;
			TextInputWidget(
				const std::string& label,
				float inputTextFieldWidth,
				const std::string& defaultText = "",
				bool editable = true,
				uint32_t bufferSize = 128);

			virtual bool draw() override;

		private:
			std::string _text;
			bool _editable;
			uint32_t _bufferSize;
			float _inputTextFieldWidth;
	};
}