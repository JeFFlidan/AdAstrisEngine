#include "platform_process.h"
#include "profiler/logger.h"

using namespace ad_astris;

std::unique_ptr<PlatformResourcePtr> PlatformProcess::load_dll(const std::string& filename)
{
#ifdef _WIN32
	std::unique_ptr<HModulePtr> module = std::make_unique<HModulePtr>();
	HMODULE handle = LoadLibraryA(filename.c_str());
	if (handle == NULL)
		LOG_FATAL("PlatformProcess::load_dll(): Failed to load {}", filename)
	module->reset(handle);
	return module;
#endif
}

void* PlatformProcess::get_proc_addr(const PlatformResourcePtr& dllHandle, const std::string& procName)
{
#ifdef _WIN32
	const HModulePtr& module = static_cast<const HModulePtr&>(dllHandle);
	return GetProcAddress(module, procName.c_str());
#endif
}
