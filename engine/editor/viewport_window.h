#pragma once

#include "common.h"
#include "ui_core/common.h"
#include <functional>

namespace ad_astris::editor::impl
{
	class ViewportWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			ViewportWindow(UIWindowInitContext& initContext, std::function<uint64_t()> callback);
			virtual void draw_window(void* data) override;

		private:
			std::function<uint64_t()> _textureCallback{ nullptr };
			uint64_t _textureIndex;
			ImVec2 _viewportExtent{ 0, 0 };
	};
}