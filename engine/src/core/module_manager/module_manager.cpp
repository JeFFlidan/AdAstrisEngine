#include "module.h"
#include "module_manager.h"
#include "core/config_base.h"
#include "profiler/logger.h"
#include "file_system/utils.h"
#include "core/global_objects.h"
#include "core/platform_process.h"

using namespace ad_astris;

ModuleManager::ModuleManager()
{
	io::URI path = io::Utils::get_absolute_path_to_file(FILE_SYSTEM()->get_engine_root_path(), "configs/modules.ini");
	
	Config config;
	config.load_from_file(path);

	for (auto section : config)
	{
		_engineDllPathByModulePseudonym[section.get_name()] = section.get_option_value<std::string>("Path");
	}
}

void ModuleManager::cleanup()
{
	for (auto& moduleInfo : _loadedModules)
	{
		moduleInfo.second.module->shutdown_module(this);
		destroy_module(moduleInfo.second.module);
	}
	_loadedModules.clear();
}

void ModuleManager::load_project_modules_config()
{
	io::URI path = io::Utils::get_absolute_path_to_file(FILE_SYSTEM()->get_project_root_path(), "configs/modules.ini");
	Config config;
	config.load_from_file(path);
	for (auto section : config)
	{
		_projectDllPathByModulePseudonym[section.get_name()] = section.get_option_value<std::string>("Path");
	}
}

typedef IModule* (*RegisterModuleFunc)();

IModule* ModuleManager::load_module(const std::string& moduleName)
{
	auto moduleInfoFromMap = _loadedModules.find(moduleName);
	if (moduleInfoFromMap != _loadedModules.end())
		return moduleInfoFromMap->second.module;
	
	std::string modulePath = get_module_path(moduleName);
	
	ModuleInfo& moduleInfo = _loadedModules[moduleName];
	moduleInfo.dllHandle = PlatformProcess::load_dll(modulePath);
	void* registerFunc = PlatformProcess::get_proc_addr(*moduleInfo.dllHandle, "register_module");
	if (registerFunc != nullptr)
	{
		moduleInfo.module = reinterpret_cast<RegisterModuleFunc>(registerFunc)();
	}
	else
	{
		moduleInfo.module = new IModule();
	}
	moduleInfo.module->_name = moduleName;
	moduleInfo.module->startup_module(this);

	return moduleInfo.module;
}

void ModuleManager::unload_module(const std::string& moduleName)
{
	auto moduleInfoIt = _loadedModules.find(moduleName);
	
	if (moduleInfoIt == _loadedModules.end())
	{
		LOG_WARNING("ModuleManager::unload_module(): Failed to unload module {}", moduleName)
		return;
	}

	destroy_module(moduleInfoIt->second.module);
	_loadedModules.erase(moduleInfoIt);
}

const PlatformResourcePtr& ModuleManager::get_platform_dll_handle(const std::string& moduleName)
{
	auto it = _loadedModules.find(moduleName);
	if (it == _loadedModules.end())
	{
		load_module(moduleName);
		it = _loadedModules.find(moduleName);
	}
	return *it->second.dllHandle;
}

std::string ModuleManager::get_module_path(const std::string& moduleName)
{
	auto it = _engineDllPathByModulePseudonym.find(moduleName);
    if (it == _engineDllPathByModulePseudonym.end())
    {
    	auto it2 = _projectDllPathByModulePseudonym.find(moduleName);
    	if (it2 == _projectDllPathByModulePseudonym.end())
    	{
    		LOG_FATAL("ModuleManager::get_module_relative_path(): There is no module with pseudonym {}", moduleName)
    	}
    	else
    	{
    		return io::Utils::get_absolute_path_to_file(FILE_SYSTEM()->get_project_root_path(), it2->second).c_str();
    	}
    }
	return io::Utils::get_absolute_path_to_file(FILE_SYSTEM()->get_engine_root_path(), it->second).c_str();
}

void ModuleManager::destroy_module(IModule* module)
{
	auto proc = module->get_procedure<void()>("destroy_module");
	if (proc.is_valid())
	{
		proc();
	}
	else
	{
		delete module;
		module = nullptr;
	}
}
