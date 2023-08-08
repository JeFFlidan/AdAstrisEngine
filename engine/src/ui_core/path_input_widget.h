#pragma once

#include "internal/base_widget.h"

namespace ad_astris::uicore
{
	class PathInputWidget : public internal::BaseWidget
	{
		public:
			PathInputWidget(PathInputType inputType);
	};
}
