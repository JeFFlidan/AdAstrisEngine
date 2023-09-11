#include "event_manager.h"
#include "profiler/logger.h"

#include <thread>

using namespace ad_astris;
using namespace events;

EventManager::EventManager()
{
	
}

void EventManager::unsubscribe(uint64_t eventID, const std::string& eventHandlerTypeName)
{
	std::lock_guard<std::mutex> locker(_handlersByEventIDMutex);
	auto it = _handlersByEventID.find(eventID);

	if (it == _handlersByEventID.end())
	{
		LOG_ERROR("EventManager::unsubscribe(): EventManager doesn't know event with ID {}", eventID)
		return;
	}

	auto& handlers = it->second;

	for (auto beginIt = handlers.begin(); beginIt != handlers.end(); ++beginIt)
	{
		if (beginIt->get()->get_type() == eventHandlerTypeName)
		{
			handlers.erase(beginIt);
			return;
		}
	}
}

void EventManager::trigger_event(IEvent& event)
{
	std::lock_guard<std::mutex> locker(_eventQueueMutex);
	auto it = _handlersByEventID.find(event.get_type_id());
	if (it == _handlersByEventID.end())
		return;

	for (auto& handler : it->second)
	{
		handler->execute(event);
	}
	event.cleanup();
}

void EventManager::dispatch_events()
{
	while (!_eventsQueue.empty())
	{
		IEvent* event = _eventsQueue.front().get();
		auto it = _handlersByEventID.find(event->get_type_id());
		if (it == _handlersByEventID.end())
		{
			event->cleanup();
			_eventsQueue.pop();
			continue;
		}
		
		auto& handlers = it->second;
		for (auto& handler : handlers)
		{
			handler->execute(*event);
		}

		event->cleanup();
		_eventsQueue.pop();
	}
}


void EventManagerTests::main_loop()
{
	LOG_INFO("Before first thread")
	std::thread thread1([&]() { test1(); });
	LOG_INFO("Before second thread")
	std::thread thread2([&]() { test2(); });
	LOG_INFO("Before third thread")
	std::thread thread3([&](){ test3(); });

	LOG_INFO("Before first join")
	thread1.join();
	LOG_INFO("Before second join")
	thread2.join();
	LOG_INFO("Before third join")
	thread3.join();

	LOG_INFO("Before dispatch")
	eventManager.dispatch_events();
	LOG_INFO("Finish testing EventManager")
}

void EventManagerTests::test1()
{
	EventDelegate<Event1> delegate = [](Event1& event)
	{
		LOG_INFO("EXECUTE EVENT1 DELEGATE FROM METHOD TEST1")
	};
	
	eventManager.subscribe(Event1::get_type_id_static(), delegate);

	Event1 event1;
	eventManager.enqueue_event(event1);
}

void EventManagerTests::test2()
{
	EventDelegate<Event2> delegate = [](Event2& event)
	{
		LOG_INFO("EXECUTE EVENT2 DELEGATE FROM METHOD TEST2")
	};

	EventDelegate<Event3> delegate3 = [](Event3& event)
	{
		LOG_INFO("EXECUTE EVENT3 DELEGATE FROM METHOD TEST2")
	};
	
	eventManager.subscribe(Event2::get_type_id_static(), delegate);
	
	eventManager.subscribe(Event3::get_type_id_static(), delegate3);
	
	Event2 event2;
	eventManager.enqueue_event(event2);
}

void EventManagerTests::test3()
{
	EventDelegate<Event3> delegate3 = [](Event3& event)
	{
		LOG_INFO("EXECUTE EVENT3 DELEGATE FROM METHOD TEST3")
	};
	
	eventManager.subscribe(Event3::get_type_id_static(), delegate3);

	Event3 event3;
	eventManager.trigger_event(event3);
}
