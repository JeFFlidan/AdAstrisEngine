#pragma once

#include "module.h"
#include "file_system/file_system.h"

#include <unordered_map>
#include <string>

struct HINSTANCE__;
typedef long long int (*DLL_FUNC_PTR)();		// I must think why including windows.h in this file causes compile errors

namespace ad_astris
{
	struct ModuleInfo
	{
		typedef IModule* RegisterModuleFunc();

		HINSTANCE__* moduleHandler;
		DLL_FUNC_PTR registerFuncHandler;
		RegisterModuleFunc* registerModuleFunc{ nullptr };
		IModule* module{ nullptr };
	};

	class ModuleManager
	{
		public:
			ModuleManager(io::FileSystem* fileSystem);

			void cleanup();
		
			DLL_FUNC_PTR load_third_party_module(const std::string& moduleName, const std::string& registerFuncName);
			IModule* load_module(const std::string& moduleName);

			template<typename T>
			T* load_module(const std::string& fileName)
			{
				return reinterpret_cast<T*>(load_module(fileName));
			}

			void unload_module(const std::string& moduleName);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			std::unordered_map<std::string, std::string> _dllPathByModulePseudonym;
			std::unordered_map<std::string, ModuleInfo> _loadedModules;
			std::string _rootPath;

			struct ModuleInternalState
			{
				HINSTANCE__* moduleHandler;
				DLL_FUNC_PTR registerFunc;
			};
		
			void load_module_internal(
				const std::string& moduleName,
				const std::string& registerFuncName,
				ModuleInternalState& internalState);
	};
}