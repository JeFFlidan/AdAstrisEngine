#include "object.h"

using namespace ad_astris;

io::URI ecore::Object::get_path()
{
	return _path;
}

ecore::ObjectName* ecore::Object::get_name()
{
	return &_name;
}

bool ecore::Object::rename(io::FileSystem* fileSystem, std::string newName)
{
	// TODO
	return true;
}

void ecore::Object::rename_file_on_disc(io::URI& oldPath, io::URI& newPath)
{
	// TODO
}

void ecore::Object::rename_in_engine(ObjectName& newName)
{
	// TODO
}
