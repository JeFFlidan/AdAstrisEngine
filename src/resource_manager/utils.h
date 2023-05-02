#pragma once

#include "resource_formats.h"
#include <string>

namespace ad_astris::resource
{
	class Utils
	{
		public:
			static std::string get_str_resource_type(ResourceType type);
			static ResourceType get_enum_resource_type(std::string type);
	};
}