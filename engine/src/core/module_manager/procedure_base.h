#pragma once

#include <string>
#include <functional>

namespace ad_astris
{
	class IModule;
	
	class ProcedureBase 
	{
		public:
			ProcedureBase(const std::string& moduleName, const std::string& procedureName);
			virtual ~ProcedureBase() = default;

			void* get_address() const;
			std::string get_name() const { return _name; }
			virtual bool is_valid();
		
		protected:
			std::string _name;
			std::function<void*(const std::string&)> _loadFunc{ nullptr };
	};
}