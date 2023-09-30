#pragma once

#include "common.h"

namespace ad_astris::editor::impl
{
	class ModelCreationWindow : public UIWindowInternal
	{
		public:
			ModelCreationWindow(UIWindowInitContext& initContext);
			void draw();
			bool need_drawing() { return _needDrawing; }
			void set_drawing_state(bool state) { _needDrawing = state; }

		private:
			ResourceDesc _modelDesc;
			ResourceDesc _materialDesc;
			bool _needDrawing{ false };
	};
}