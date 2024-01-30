#pragma once

#include "custom_objects_to_json.h"
#include "profiler/logger.h"
#include "reflection.h"

namespace ad_astris::serialization
{
	template<typename T>
	nlohmann::json serialize_to_json(T& object)
	{
		nlohmann::json objectJson;
		refl::util::for_each(refl::reflect(object).members, [&](auto member)
		{
			if constexpr (refl::descriptor::is_readable(member) && refl::descriptor::has_attribute<SerializableFields>(member))
			{
				objectJson[refl::descriptor::get_display_name(member)] = member(object);
			}
		});
		return objectJson;																							
	}

	template<typename T>
	void deserialize_from_json(const nlohmann::json& json, T& object)
	{
		refl::util::for_each(refl::reflect(object).members, [&](auto member)
		{
			member(object, json[refl::descriptor::get_display_name(member)]);
		});
	}
}
