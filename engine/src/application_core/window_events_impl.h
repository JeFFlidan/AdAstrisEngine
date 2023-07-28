#pragma once

#include "enums.h"

namespace ad_astris::acore::impl
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
	
	class MouseMoveEvent
	{
		public:
			MouseMoveEvent() = default;
			MouseMoveEvent(int32_t xPos, int32_t yPos) : _xPos(xPos), _yPos(yPos) { }

			int32_t get_x_position()
			{
				return _xPos;
			}

			int32_t get_y_position()
			{
				return _yPos;
			}

		protected:
			int32_t _xPos{ 0 };
			int32_t _yPos{ 0 };
	};
		
	class MouseButtonEvent : public MouseMoveEvent
	{
		public:
			MouseButtonEvent() = default;
			MouseButtonEvent(MouseButton mouseButton, int32_t xPos, int32_t yPos)
				: MouseMoveEvent(xPos, yPos), _mouseButton(mouseButton) { }
				
			MouseButton get_button_state()
			{
				return _mouseButton;
			}

		protected:
			MouseButton _mouseButton{ MouseButton::UNKNOWN };
	};

	class MouseMoveWithPressedButtonEvent : public MouseMoveEvent
	{
		public:
			MouseMoveWithPressedButtonEvent() = default;
		
			void set_event_is_valid()
			{
				_isEventValid = true;
			}

			bool is_event_valid()
			{
				return _isEventValid;
			}

			void set_x_position(int32_t xPos)
			{
				_xPos = xPos;
			}

			void set_y_position(int32_t yPos)
			{
				_yPos = yPos;
			}

		private:
			bool _isEventValid{ false };
	};
}