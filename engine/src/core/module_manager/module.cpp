#include "module.h"
#include "core/global_objects.h"

using namespace ad_astris;

void IModule::set_global_objects() const
{
	auto proc = get_procedure<void(GlobalObjectContext*)>("set_global_objects");
	if (proc.is_valid())
		proc(GlobalObjects::get_global_object_context());
}
