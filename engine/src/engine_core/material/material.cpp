#include "material.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace ecore;

Material::Material(const MaterialInfo& info, ObjectName* name) : _info(info)
{
	_name = name;
}

void Material::serialize(io::File* file)
{
	LOG_WARNING("Material::serialize(): NOT IMPLEMENTED")
}

void Material::deserialize(io::File* file, ObjectName* objectName)
{
	LOG_WARNING("Material::deserialize(): NOT IMPLEMENTED")
}

void Material::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}
