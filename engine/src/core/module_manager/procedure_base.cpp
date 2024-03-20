#include "procedure_base.h"
#include "core/global_objects.h"
#include "core/platform_process.h"

using namespace ad_astris;

ProcedureBase::ProcedureBase(const std::string& moduleName, const std::string& procedureName)
	: _name(procedureName)
{
	_loadFunc = [moduleName](const std::string& procName)->void*
	{
#ifdef _WIN32
		return PlatformProcess::get_proc_addr(MODULE_MANAGER()->get_platform_dll_handle(moduleName), procName);
#endif
	};
}

void* ProcedureBase::get_address() const
{
	return _loadFunc(_name);
}

bool ProcedureBase::is_valid()
{
	if (_loadFunc(_name) != NULL)
		return true;
	return false;
}
