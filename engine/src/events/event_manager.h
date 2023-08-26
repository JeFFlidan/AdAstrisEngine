#pragma once

#include "event_handler.h"
#include "profiler/logger.h"

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

			template<typename EventType>
			void subscribe(uint64_t eventID, EventDelegate<EventType>& eventDelegate)
			{
				auto it = _handlersByEventID.find(eventID);
				std::unique_ptr<EventHandler<EventType>> eventHandler(new EventHandler<EventType>(eventDelegate));

				if (it != _handlersByEventID.end())
				{
					for (auto& handler : _handlersByEventID[eventID])
					{
						if (handler->get_type() == eventHandler->get_type())
						{
							LOG_ERROR("EventManager::subscribe(): Engine subscribed the event to the same event handler")
							return;
						}
					}
				}

				_handlersByEventID[eventID].emplace_back(std::move(eventHandler));
			}
		
			void unsubscribe(uint64_t eventID, const std::string& eventHandlerTypeName);

			template<typename CustomEvent>
			void enqueue_event(CustomEvent& event)
			{
				std::scoped_lock<std::mutex> locker(_eventQueueMutex);
				_eventsQueue.emplace(new CustomEvent(event));
			}
		
			void trigger_event(IEvent& event);
			void dispatch_events();
		
		private:
			std::queue<std::unique_ptr<IEvent>> _eventsQueue;
			std::mutex _eventQueueMutex;
			std::unordered_map<uint64_t, std::vector<std::unique_ptr<IEventHandler>>> _handlersByEventID;
			std::mutex _handlersByEventIDMutex;
	};

	class Event1 : public IEvent
	{
		public:
			EVENT_TYPE_DECL(Event1)

			uint32_t xPos{ 0 };
	};

	class Event2 : public IEvent
	{
		public:
			EVENT_TYPE_DECL(Event2)
			uint32_t yPos{ 0 };
			uint32_t zPos{ 1 };
	};

	class Event3 : public IEvent
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