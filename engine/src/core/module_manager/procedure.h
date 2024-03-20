#pragma once

#include "procedure_base.h"

namespace ad_astris
{
	template<typename T>
	class Procedure : public ProcedureBase
	{
		public:
			Procedure(const std::string& moduleName, const std::string& procedureName)
				: ProcedureBase(moduleName, procedureName) { }
		
			operator T*()
			{
				if (!_func)
					_func = static_cast<T*>(get_address());
				return _func;
			}

			virtual bool is_valid() override
			{
				if (!_func)
				{
					void* address = get_address();
					if (address != nullptr)
					{
						_func = static_cast<T*>(get_address());
						return true;
					}
					return false;
				}
				return true;
			}

		private:
			T* _func{ nullptr };
	};
}
