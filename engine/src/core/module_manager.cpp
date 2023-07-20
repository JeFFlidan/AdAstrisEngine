#include "config_base.h"
#include "module_manager.h"
#include "profiler/logger.h"
#include "file_system/utils.h"

#include <windows.h>

using namespace ad_astris;

ModuleManager::ModuleManager(io::FileSystem* fileSystem)
{
	io::URI path = io::Utils::get_absolute_path_to_file(fileSystem, "configs/modules.ini");
	
	Config config;
	config.load_from_file(path);

	for (auto section : config)
	{
		_dllPathByModulePseudonym[section.get_name()] = section.get_option_value<std::string>("Path");
	}
}

IModule* ModuleManager::load_module(const std::string& moduleName)
{
	LOG_INFO("Start loading module")
	auto moduleInfoFromMap = _loadedModules.find(moduleName);
	if (moduleInfoFromMap != _loadedModules.end())
		return moduleInfoFromMap->second.module;

	auto itFromDllPathByModulePseudonym = _dllPathByModulePseudonym.find(moduleName);
	if (itFromDllPathByModulePseudonym == _dllPathByModulePseudonym.end())
		LOG_FATAL("ModuleManager::load_module(): There is no module with pseudonym {}", moduleName)

	std::string modulePath = itFromDllPathByModulePseudonym->second;
	
	ModuleInfo moduleInfo;
	HMODULE hmodule = LoadLibraryA(modulePath.c_str());
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
	moduleInfo.module->startup_module(this);

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
