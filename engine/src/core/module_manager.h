#pragma once

#include "module.h"
#include "file_system/file_system.h"

#include <unordered_map>
#include <string>

struct HINSTANCE__;

namespace ad_astris
{
	struct ModuleInfo
	{
		typedef IModule* RegisterModuleFunc();

		HINSTANCE__* moduleHandler;
		RegisterModuleFunc* registerPluginFunc;
		IModule* module;
	};
	
	class ModuleManager
	{
		public:
			ModuleManager(io::FileSystem* fileSystem);
			IModule* load_module(const std::string& moduleName);

			template<typename T>
			T* load_module(const std::string& fileName)
			{
				return reinterpret_cast<T*>(load_module(fileName));
			}

			void unload_module(const std::string& moduleName);
		
		private:
			std::unordered_map<std::string, std::string> _dllPathByModulePseudonym;
			std::unordered_map<std::string, ModuleInfo> _loadedModules;
			std::string _rootPath;
	};
}