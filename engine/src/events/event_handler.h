#pragma once

#include "event.h"
#include <functional>
#include <string>

namespace ad_astris::events
{
	template<typename EventType>
	using EventDelegate = std::function<void(EventType& event)>;
	
	class IEventHandler
	{
		public:
			virtual ~IEventHandler() { }
		
			void execute(IEvent& event)
			{
				internal_execute(event);
			}

			virtual std::string get_type() = 0;

		protected:
			virtual void internal_execute(IEvent& event) = 0;
	};

	template<typename EventType>
	class EventHandler : public IEventHandler
	{
		public:
			explicit EventHandler(EventDelegate<EventType>& eventDelegate)
				: _eventDelegate(eventDelegate), _type(_eventDelegate.target_type().name()) { }

			virtual std::string get_type() override
			{
				return _type;
			}

		private:
			EventDelegate<EventType> _eventDelegate;
			std::string _type;

			virtual void internal_execute(IEvent& event) override
			{
				if (EventType::get_type_id_static() == event.get_type_id())
					_eventDelegate(static_cast<EventType&>(event));
			}
	};
}