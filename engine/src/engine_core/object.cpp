#include "object.h"
#include "file_system/utils.h"
#include "profiler/logger.h"
#include <cstdio>

using namespace ad_astris;

void ecore::Object::set_path(const io::URI& path)
{
	_path = path;
}

io::URI ecore::Object::get_path()
{
	return _path;
}

ecore::ObjectName* ecore::Object::get_name()
{
	return _name;
}

bool ecore::Object::rename(const std::string& newName)
{
	io::URI oldPath = _path;
	ObjectName oldName = *_name;
	
	if (io::Utils::is_absolute(newName.c_str()))
	{
		_name->change_name(io::Utils::get_file_name(newName.c_str()).c_str());
		std::string extension = io::Utils::get_file_extension(_path);
		_path = newName.c_str();
		io::Utils::replace_filename(_path, _name->get_full_name() + '.' + extension);
		
		if (std::rename(oldPath.c_str(), _path.c_str()) != 0)
		{
			LOG_ERROR("Object::rename(): Can't change filename from {} to {}", oldPath.c_str(), _path.c_str())
			*_name = oldName;
			_path = oldPath;
			return false;
		}
	}
	else if (io::Utils::has_extension(newName.c_str()))
	{
		LOG_ERROR("Object::rename(): Can't rename file using relative path {}", newName)
		return false;
	}
	else
	{
		_name->change_name(newName.c_str());
		io::Utils::replace_filename(_path, _name->get_full_name());
		if (std::rename(oldPath.c_str(), _path.c_str()) != 0)
		{
			LOG_ERROR("Object::rename(): Can't change filename from {} to {}", oldPath.c_str(), _path.c_str())
			*_name = oldName;
			_path = oldPath;
			return false;
		}
	}
	LOG_INFO("Object::rename(): Changed filename from {} to {}", oldPath.c_str(), _path.c_str())
	return true;
}
