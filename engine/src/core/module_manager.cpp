#include "profiler/logger.h"

#include <windows.h>

#include "module_manager.h"

using namespace ad_astris;

IModule* ModuleManager::load_module(const std::string& moduleName)
{
	LOG_INFO("Start loading module")
	auto moduleInfoFromMap = _loadedModules.find(moduleName);
	if (moduleInfoFromMap != _loadedModules.end())
		return moduleInfoFromMap->second.module;
	
	ModuleInfo moduleInfo;
	HMODULE hmodule = LoadLibraryA(moduleName.c_str());
	if (hmodule == NULL)
	{
		LOG_FATAL("ModuleManager::load_module(): Failed to load module {}", moduleName)
	}

	moduleInfo.moduleHandler = hmodule;
	
	FARPROC registerFuncPtr = GetProcAddress(hmodule, "register_module");
	moduleInfo.registerPluginFunc = reinterpret_cast<ModuleInfo::RegisterModuleFunc*>(registerFuncPtr);

	if (moduleInfo.registerPluginFunc == NULL)
	{
		LOG_WARNING("EXTERN C FUNC IS INVALID")
	}
	
	moduleInfo.module = moduleInfo.registerPluginFunc();

	if (!moduleInfo.module)
		LOG_WARNING("MODULE IS INVALID")

	_loadedModules[moduleName] = moduleInfo;
	
	return moduleInfo.module;
}

void ModuleManager::unload_module(const std::string& moduleName)
{
	auto moduleInfoIt = _loadedModules.find(moduleName);
	
	if (moduleInfoIt == _loadedModules.end())
	{
		LOG_WARNING("ModuleManager::unload_module(): Failed to unload module {}", moduleName)
	}

	FreeModule(moduleInfoIt->second.moduleHandler);
}
