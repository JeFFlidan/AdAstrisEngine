#pragma once

#include "custom_objects_to_json.h"
#include "reflection.h"
#include "attributes.h"
#include "profiler/logger.h"

namespace ad_astris::serialization
{
	template<typename T>
	nlohmann::json serialize_to_json(T& object)
	{
		nlohmann::json objectJson;
		Reflector::for_each<T>([&](auto field)
		{
			if constexpr (Reflector::has_attribute<Serializable>(field))
				objectJson[Reflector::get_name(field)] = field(object);
		});
		return objectJson;																							
	}

	template<typename T>
	void deserialize_from_json(const nlohmann::json& json, T& object)
	{
		Reflector::for_each<T>([&](auto field)
		{
			if constexpr (Reflector::has_attribute<Serializable>(field))
				field(object, json[Reflector::get_name(field)]);
		});
	}
}
