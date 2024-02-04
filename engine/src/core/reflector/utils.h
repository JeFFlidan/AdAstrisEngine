#pragma once

#include <json/json.hpp>
#include <cstring>
#include <string>

#if defined(__MINGW32__)
#define CORE_FUNC_NAME_FRONT(type, name) ((sizeof(#type) + sizeof(" ad_astris::() [with T = ") + sizeof(#name)) - 3u)
#define CORE_FUNC_NAME_BACK (sizeof("]") - 1u)
#define CORE_FUNC_NAME __PRETTY_FUNCTION__
#endif

#if defined(_MSC_VER)
#define CORE_FUNC_NAME_FRONT(type, name) ((sizeof(#type) + sizeof("__cdecl ad_astris::") + sizeof(#name) + sizeof("<struct ")) - 3u)
#define CORE_FUNC_NAME_BACK (sizeof(">(void)"))
#define CORE_FUNC_NAME __FUNCSIG__
#endif

#define CORE_FUNC_TYPE_LEN(type, name, str)\
(sizeof(str) - (CORE_FUNC_NAME_FRONT(type, name) + CORE_FUNC_NAME_BACK))

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
}
