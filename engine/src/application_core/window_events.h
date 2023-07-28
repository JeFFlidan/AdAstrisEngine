#pragma once

#include "enums.h"
#include "window_events_impl.h"
#include "events/event.h"
#include "events/event_manager.h"

namespace ad_astris::acore
{
	class WindowResizedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(WindowResizedEvent)
			WindowResizedEvent(uint32_t width, uint32_t height) : _width(width), _height(height) { }

			uint32_t get_width()
			{
				return _width;
			}

			uint32_t get_height()
			{
				return _height;
			}
		
		private:
			uint32_t _width{ 0 };
			uint32_t _height{ 0 };
	};

	class KeyDownEvent : public impl::KeyEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(KeyDownEvent)
	};

	class KeyUpEvent : public impl::KeyEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(KeyUpEvent)
	};

	class MouseButtonDownEvent : public impl::MouseButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseButtonDownEvent)
			MouseButtonDownEvent() = default;
			MouseButtonDownEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
				: MouseButtonEvent(mouseButton, xPos, yPos) { }
	};

	class MouseButtonUpEvent : public impl::MouseButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseButtonUpEvent)
			MouseButtonUpEvent() = default;
			MouseButtonUpEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
				: MouseButtonEvent(mouseButton, xPos, yPos) { }
	};

	class MouseMoveWithLeftButtonPressedEvent : public impl::MouseMoveWithPressedButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseMoveWithLeftButtonPressedEvent)
	};

	class MouseMoveWithRightButtonPressedEvent : public impl::MouseMoveWithPressedButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseMoveWithRightButtonPressedEvent)
	};

	namespace impl
	{
		struct WindowEvents
		{
			KeyDownEvent keyDownEvent;
			KeyUpEvent keyUpEvent;
			MouseButtonDownEvent mouseButtonDownEvent;
			MouseButtonUpEvent mouseButtonUpEvent;
			MouseMoveWithLeftButtonPressedEvent mouseMoveLeftButton;
			MouseMoveWithRightButtonPressedEvent mouseMoveRightButton;

			void enqueue_events(events::EventManager* eventManager);
		};
	}
}
