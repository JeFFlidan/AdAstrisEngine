#pragma once

#include "common.h"
#include "ui_core/common.h"

namespace ad_astris::editor::impl
{
	class PropertiesWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			PropertiesWindow(UIWindowInitContext& initContext) : UIWindowInternal(initContext) { }
			virtual void draw_window(void* data) override;
	};
}