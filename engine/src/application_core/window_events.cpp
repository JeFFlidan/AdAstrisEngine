#include "window_events.h"

using namespace ad_astris::acore::impl;

void WindowEvents::enqueue_events(events::EventManager* eventManager)
{
	if (keyDownEvent.get_keys_state() != Key::UNKNOWN)
		eventManager->enqueue_event(keyDownEvent);

	if (keyUpEvent.get_keys_state() != Key::UNKNOWN)
		eventManager->enqueue_event(keyUpEvent);

	if (mouseButtonDownEvent.get_button_state() != MouseButton::UNKNOWN)
		eventManager->enqueue_event(mouseButtonDownEvent);

	if (mouseButtonUpEvent.get_button_state() != MouseButton::UNKNOWN)
		eventManager->enqueue_event(mouseButtonUpEvent);

	if (mouseMoveLeftButton.is_event_valid())
		eventManager->enqueue_event(mouseMoveLeftButton);

	if (mouseMoveRightButton.is_event_valid())
		eventManager->enqueue_event(mouseMoveRightButton);
}
