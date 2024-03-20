#pragma once

#include "platform_resource.h"
#include <string>
#include <memory>

namespace ad_astris
{
	class PlatformProcess
	{
		public:
			static std::unique_ptr<PlatformResourcePtr> load_dll(const std::string& filename);
			static void* get_proc_addr(const PlatformResourcePtr& dllHandle, const std::string& procName);
	};
}