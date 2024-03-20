#pragma once

#include "module.h"
#include "core/platform_resource.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace ad_astris
{
	struct ModuleInfo
	{
		IModule* module{ nullptr };
		std::unique_ptr<PlatformResourcePtr> dllHandle{ nullptr };
	};

	class ModuleManager
	{
		public:
			ModuleManager();

			void cleanup();

			void load_project_modules_config();
			IModule* load_module(const std::string& moduleName);

			template<typename T>
			T* load_module(const std::string& moduleName)
			{
				return reinterpret_cast<T*>(load_module(moduleName));
			}

			void unload_module(const std::string& moduleName);
		
			const PlatformResourcePtr& get_platform_dll_handle(const std::string& moduleName);
		
		private:
			std::unordered_map<std::string, std::string> _engineDllPathByModulePseudonym;
			std::unordered_map<std::string, std::string> _projectDllPathByModulePseudonym;
			std::unordered_map<std::string, ModuleInfo> _loadedModules;

			std::string get_module_path(const std::string& moduleName);
			void destroy_module(IModule* module);
	};

#define DLL_EXPORT __declspec(dllexport)

#define DECL_MODULE_FUNCS(ModuleName, ModuleImpl)												\
	ModuleImpl* MODULE{ nullptr };																\
	extern "C" DLL_EXPORT IModule* register_module()											\
	{																							\
		if (!MODULE)																			\
		{																						\
			MODULE = new ModuleImpl();															\
			LOG_INFO("{}::register_module(): Module successfully registered", #ModuleName)		\
		}																						\
		else																					\
		{																						\
			LOG_WARNING("{}::register_module(): Module was previously registered", #ModuleName)	\
		}																						\
		return reinterpret_cast<IModule*>(MODULE);												\
	}																							\
	extern "C" DLL_EXPORT void destroy_module()													\
	{																							\
		delete MODULE;																			\
		MODULE = nullptr;																		\
		LOG_INFO("{}::destroy_module(): Module successfully destroyed", #ModuleName)			\
	}																							\
	extern "C" DLL_EXPORT void set_global_objects(GlobalObjectContext* context)					\
	{																							\
		GlobalObjects::set_global_object_context(context);										\
	}
}