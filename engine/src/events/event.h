#pragma once

#include "core/compile_time_hash.h"

namespace ad_astris::events
{
#define EVENT_TYPE_HASH(x) compile_time_fnv1(#x)

#define EVENT_TYPE_DECL(x)									\
	enum class EventType : uint64_t							\
	{														\
		TYPE_ID = EVENT_TYPE_HASH(x)						\
	};														\
	static inline constexpr uint64_t get_type_id_static()	\
	{														\
		return static_cast<uint64_t>(EventType::TYPE_ID);	\
	}														\
	virtual uint64_t get_type_id() override					\
	{														\
		return static_cast<uint64_t>(EventType::TYPE_ID);	\
	}

	class IEvent
	{
		public:
			virtual ~IEvent() { }
			virtual uint64_t get_type_id() = 0; 
	};
}