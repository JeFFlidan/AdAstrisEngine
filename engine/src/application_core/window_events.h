#pragma once

#include "enums.h"
#include "core/math_base.h"
#include "events/event.h"
#include "events/event_manager.h"
#include "win_api_utils.h"

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

	class DeltaTimeUpdateEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(DeltaTimeEvent)

			DeltaTimeUpdateEvent(float deltaTime) : _deltaTime(deltaTime) { }
			float get_delta_time() { return _deltaTime; }

		private:
			float _deltaTime;
	};

	class InputEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(InputEvent)

			void set_mouse_state(MouseState mouseState){ _mouseState = mouseState; }
			void set_position(XMFLOAT2 position) { _mouseState.position = position; }
		
			bool is_mouse_button_pressed(MouseButton mouseButton)
			{
				switch (mouseButton)
				{
					case MouseButton::RIGHT:
						return impl::WinApiUtils::is_key_down(VK_RBUTTON);
					case MouseButton::LEFT:
						return impl::WinApiUtils::is_key_down(VK_LBUTTON);
					case MouseButton::MIDDLE:
						return impl::WinApiUtils::is_key_down(VK_MBUTTON);
				}
			}

			XMFLOAT2 get_position() { return _mouseState.position; }
			XMFLOAT2 get_delta_position() { return _mouseState.deltaPosition; }
		
			bool is_key_pressed(Key key)
			{
				uint8_t keyCode = impl::WinApiUtils::parse_key(key);
				return impl::WinApiUtils::is_key_down(keyCode) || impl::WinApiUtils::is_key_toggle(keyCode);
			}

			void set_viewport_state(bool isHovered) { _isViewportHovered = isHovered; }
			bool is_viewport_hovered() { return _isViewportHovered; }
		
		private:
			MouseState _mouseState;
			bool _isViewportHovered{ false };
	};
}
