#pragma once

#include "ui_core/common.h"

namespace ad_astris::editor::impl
{
	class ContentBrowserWindow : public uicore::IUIWindow
	{
		public:
			virtual void draw_window(void* data) override;
	};
}