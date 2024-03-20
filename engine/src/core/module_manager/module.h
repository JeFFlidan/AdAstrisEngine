#pragma once

#include "procedure.h"
#include <string>

namespace ad_astris
{
	class ModuleManager;

	class IModule
	{
		friend ModuleManager;
		
		public:
			virtual ~IModule() { }

			virtual void startup_module(ModuleManager* moduleManager) { }
			virtual void shutdown_module(ModuleManager* moduleManager) { }

			template<typename T>
			Procedure<T> get_procedure(const std::string& procedureName) const
			{
				return Procedure<T>(_name, procedureName);
			}

			ProcedureBase get_procedure(const std::string& procedureName) const
			{
				return ProcedureBase(_name, procedureName);
			}

			void set_global_objects() const;
		
		private:
			std::string _name;
	};
}