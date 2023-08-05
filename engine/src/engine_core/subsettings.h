#pragma once

#include "core/compile_time_hash.h"
#include "core/config_base.h"

namespace ad_astris::ecore
{
#define SUBSETTINGS_TYPE_HASH(x) compile_time_fnv1(#x)

#define SUBSETTINGS_TYPE_DECL(x)								\
	enum class SubsettingsType : uint64_t						\
	{															\
		TYPE_ID = SUBSETTINGS_TYPE_HASH(x)						\
	};															\
	static inline constexpr uint64_t get_type_id_static()		\
	{															\
		return static_cast<uint64_t>(SubsettingsType::TYPE_ID);	\
	}															\
	virtual uint64_t get_type_id() override						\
	{															\
		return static_cast<uint64_t>(SubsettingsType::TYPE_ID);	\
	}
	
	class ISubsettings
	{
		public:
			virtual ~ISubsettings() { }
			virtual void serialize(Config& config) = 0;
			virtual void deserialize(Section& section) = 0;
			virtual uint64_t get_type_id() = 0; 
	};
}