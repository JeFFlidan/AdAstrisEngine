#pragma once

#include "enums.h"
#include "events/event.h"

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

	namespace impl
	{
		class KeyEvent
		{
			public:
				void add_key(Key key)
				{
					_keysState |= key;
				}
			
				Key get_keys_state()
				{
					return _keysState;
				}
			
			private:
				Key _keysState{ Key::UNKNOWN };
		};
	}

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

	namespace impl
	{
		class MouseButtonEvent
		{
			public:
				MouseButtonEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
					: _mouseButton(mouseButton), _xPos(xPos), _yPos(yPos) { }
			
				MouseButton get_button()
				{
					return _mouseButton;
				}

				int32_t get_x_position()
				{
					return _xPos;
				}

				int32_t get_y_position()
				{
					return _yPos;
				}

			protected:
				MouseButton _mouseButton;
				int32_t _xPos;
				int32_t _yPos;
		};
	}

	class MouseButtonDownEvent : public impl::MouseButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseButtonDownEvent)
			MouseButtonDownEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
				: MouseButtonEvent(mouseButton, xPos, yPos) { }
	};

	class MouseButtonUpEvent : public impl::MouseButtonEvent, public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MouseButtonUpEvent)
			MouseButtonUpEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
				: MouseButtonEvent(mouseButton, xPos, yPos) { }
	};
}
