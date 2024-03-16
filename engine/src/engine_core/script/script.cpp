#include "script.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace ecore;

Script::Script(const ScriptInfo& info, ObjectName* name) : _info(info)
{
	_name = name;
}

void Script::serialize(io::File* file)
{
	LOG_WARNING("Script::serialize(): NOT IMPLEMENTED")
}

void Script::deserialize(io::File* file, ObjectName* objectName)
{
	LOG_WARNING("Script::deserialize(): NOT IMPLEMENTED")
}

void Script::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}
