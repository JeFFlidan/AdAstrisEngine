#pragma once

#include "events/event.h"
#include "core/math_base.h"

namespace ad_astris::acore
{
	class ViewportResizedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ViewportResizedEvent)

			ViewportResizedEvent(float width, float height) : _width(width), _height(height) { }
			float get_width() { return _width; }
			float get_height() { return _height; }
			bool is_extent_valid() { return _width && _height; }

		private:
			float _width{ 0.0f };
			float _height{ 0.0f };
	};

	struct ViewportState
	{
		bool isHovered{ false };
		XMFLOAT2 viewportMin{ 0, 0 };
		XMFLOAT2 viewportMax{ 0, 0 };
	};

	class ViewportHoverEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ViewportHoveredEvent)
			ViewportHoverEvent(ViewportState& viewportState) : _state(viewportState) { }
			ViewportState get_viewport_state() { return _state; }

		private:
			ViewportState _state;
	};
}