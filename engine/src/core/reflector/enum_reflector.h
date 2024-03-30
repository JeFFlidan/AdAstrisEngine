#pragma once

#include <json/json.hpp>

#define REFLECT_ENUM(EnumType, ...)																\
	NLOHMANN_JSON_SERIALIZE_ENUM(EnumType, __VA_ARGS__)											\
	constexpr const char* to_string(EnumType e)													\
	{																							\
		static_assert(std::is_enum<EnumType>::value, #EnumType " must be an enum!");			\
		constexpr std::pair<EnumType, const char*> names[] = __VA_ARGS__;						\
		return names[(size_t)e].second;															\
	}																							\
	inline void from_string(const std::string& enumString, EnumType& outEnum)					\
	{																							\
		static_assert(std::is_enum<EnumType>::value, #EnumType " must be an enum!");			\
		constexpr std::pair<EnumType, const char*> names[] = __VA_ARGS__;						\
		constexpr size_t arrSize = sizeof(names) / sizeof(std::pair<EnumType, const char*>);	\
		for (size_t i = 0; i != arrSize; ++i)													\
		{																						\
			if (strcmp(names[i].second, enumString.c_str()) == 0)								\
			{																					\
				outEnum = names[i].first;														\
				break;																			\
			}																					\
		}																						\
	}
