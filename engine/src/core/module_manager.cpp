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

void ModuleManager::cleanup()
{
	for (auto& moduleInfo : _loadedModules)
	{
		FreeModule(moduleInfo.second.moduleHandler);
	}
}

DLL_FUNC_PTR ModuleManager::load_third_party_module(const std::string& moduleName, const std::string& registerFuncName)
{
	auto moduleInfoFromMap = _loadedModules.find(moduleName);
	if (moduleInfoFromMap != _loadedModules.end())
		return moduleInfoFromMap->second.registerFuncHandler;

	ModuleInternalState internalState;
	load_module_internal(moduleName, registerFuncName, internalState);

	ModuleInfo moduleInfo;
	moduleInfo.moduleHandler = internalState.moduleHandler;
	moduleInfo.registerFuncHandler = internalState.registerFunc;

	_loadedModules[moduleName] = moduleInfo;

	return moduleInfo.registerFuncHandler;
}

IModule* ModuleManager::load_module(const std::string& moduleName)
{
	auto moduleInfoFromMap = _loadedModules.find(moduleName);
	if (moduleInfoFromMap != _loadedModules.end())
		return moduleInfoFromMap->second.module;
	
	ModuleInternalState internalState;
	load_module_internal(moduleName, "register_module", internalState);
	
	ModuleInfo moduleInfo;
	moduleInfo.moduleHandler = internalState.moduleHandler;
	moduleInfo.registerFuncHandler = internalState.registerFunc;
	moduleInfo.registerModuleFunc = reinterpret_cast<ModuleInfo::RegisterModuleFunc*>(internalState.registerFunc);
	moduleInfo.module = moduleInfo.registerModuleFunc();
	moduleInfo.module->startup_module(this);

	if (!moduleInfo.module)
		LOG_WARNING("ModuleManager::load_module(): IModule* is invalid. Module {}", moduleName)

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
	_loadedModules.erase(moduleInfoIt);
}

void ModuleManager::load_module_internal(
	const std::string& moduleName,
	const std::string& registerFuncName,
	ModuleInternalState& internalState)
{
	auto itFromDllPathByModulePseudonym = _dllPathByModulePseudonym.find(moduleName);
	if (itFromDllPathByModulePseudonym == _dllPathByModulePseudonym.end())
		LOG_FATAL("ModuleManager::load_module(): There is no module with pseudonym {}", moduleName)

	std::string modulePath = itFromDllPathByModulePseudonym->second;
	
	internalState.moduleHandler = LoadLibraryA(modulePath.c_str());
	if (internalState.moduleHandler == nullptr)
	{
		LOG_FATAL("ModuleManager::load_module(): Failed to load module {}. Error: {}", moduleName, GetLastError())
	}

	internalState.registerFunc = GetProcAddress(internalState.moduleHandler, registerFuncName.c_str());
	if (internalState.registerFunc == NULL)
	{
		LOG_FATAL("ModuleManager::load_module(): Failed to get proc address of registration func. Module {}", moduleName)
	}
}
