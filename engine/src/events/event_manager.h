#pragma once

#include "event_handler.h"

#include <mutex>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ad_astris::events
{
	class EventManager
	{
		public:
			EventManager();

			void subscribe(uint64_t eventID, std::unique_ptr<IEventHandler>&& eventHandler);
			void unsubscribe(uint64_t eventID, const std::string& eventHandlerTypeName);
		
			void enqueue_event(std::unique_ptr<Event>&& event);
			void trigger_event(Event& event);
			void dispatch_events();
		
		private:
			std::queue<std::unique_ptr<Event>> _eventsQueue;
			std::mutex _eventQueueMutex;
			std::unordered_map<uint64_t, std::vector<std::unique_ptr<IEventHandler>>> _handlersByEventID;
			std::mutex _handlersByEventIDMutex;
	};

	class Event1 : public Event
	{
		public:
			EVENT_TYPE_DECL(Event1)

			uint32_t xPos{ 0 };
	};

	class Event2 : public Event
	{
		public:
			EVENT_TYPE_DECL(Event2)
			uint32_t yPos{ 0 };
			uint32_t zPos{ 1 };
	};

	class Event3 : public Event
	{
		public:
			EVENT_TYPE_DECL(Event3)

			float abc{ 10.0f };
	};

	class EventManagerTests
	{
		public:
			void main_loop();
		
		private:
			void test1();
			void test2();
			void test3();

			EventManager eventManager;
	};
}