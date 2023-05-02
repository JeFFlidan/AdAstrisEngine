#include "level.h"

using namespace ad_astris;

void ecore::Level::serialize(io::IFile* file)
{
	// TODO
}

void ecore::Level::deserialize(io::IFile* file, ObjectName* newName)
{
	// TODO
}

uint64_t ecore::Level::get_size()
{
	// TODO
}

bool ecore::Level::is_resource()
{
	return false;
}

UUID ecore::Level::get_uuid()
{
	// TODO
}

std::string ecore::Level::get_description()
{
	// TODO
}

std::string ecore::Level::get_type()
{
	return "level";
}

void ecore::Level::rename_in_engine(ObjectName& newName)
{
	
}
