#pragma once

#include "macros_utils.h"
#include "custom_objects_to_json.h"
#include <refl.hpp>
#include <json.hpp>
#include <cstring>
#include <string>
#include <iostream>

#if defined(__MINGW32__)
#define CORE_FUNC_NAME_FRONT(type, name) ((sizeof(#type) + sizeof(" ad_astris::() [with T = ") + sizeof(#name)) - 3u)
#define CORE_FUNC_NAME_BACK (sizeof("]") - 1u)
#define CORE_FUNC_NAME __PRETTY_FUNCTION__
#else
#error "Implicit component registration is not supported"
#endif

#define CORE_FUNC_TYPE_LEN(type, name, str)\
(strlen(str) - (CORE_FUNC_NAME_FRONT(type, name) + CORE_FUNC_NAME_BACK))

namespace ad_astris
{
	/** Returns the type name using a template.
	 */
	template<typename T>
	inline static const char* get_type_name()
	{
		static const size_t len = CORE_FUNC_TYPE_LEN(const char*, get_type_name, CORE_FUNC_NAME);
		static char result[len + 1] = {};
		static const size_t frontLen = CORE_FUNC_NAME_FRONT(const char*, get_type_name);
		const char* funcName = CORE_FUNC_NAME;
		memcpy(result, funcName + frontLen, len);
		static std::string strToRemoveColons(result);
		strToRemoveColons.erase(0, strToRemoveColons.find_last_of(":") + 1);
		return strToRemoveColons.c_str();
	}

	namespace uicore
	{
		struct UIField : refl::attr::usage::field {};
	}

	namespace serialization
	{
		struct SerializableFields : refl::attr::usage::field {};

		template<typename T>
		std::string serialize_to_json(T& object)
		{
			nlohmann::json objectJson;
			refl::util::for_each(refl::reflect(object).members, [&](auto member)
			{
				if constexpr (refl::descriptor::is_readable(member) && refl::descriptor::has_attribute<SerializableFields>(member))
				{
					objectJson[refl::descriptor::get_display_name(member)] = member(object);
				}
			});
			return objectJson.dump();																							
		}

		template<typename T>
		void deserialize_from_json(const std::string& jsonStr, T& object)
		{
			nlohmann::json json = nlohmann::json::parse(jsonStr);
			refl::util::for_each(refl::reflect(object).members, [&](auto member)
			{
				member(object, json[refl::descriptor::get_display_name(member)]);
			});
		}
	}
}
#define REFLECT_SERIALIZABLE_FIELD(Field) REFL_FIELD(Field, ad_astris::serialization::SerializableFields(), ad_astris::uicore::UIField())
	
#define REFLECT_SERIALIZABLE_FIELDS(Type, ...)				\
	REFL_TYPE(Type)											\
		FOR_EACH(REFLECT_SERIALIZABLE_FIELD, __VA_ARGS__)	\
	REFL_END
