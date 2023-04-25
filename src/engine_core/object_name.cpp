#include "object_name.h"
#include "profiler/logger.h"

#include <cstring>

#define MAX_NAME_LENGTH 128

using namespace ad_astris;

uint32_t ecore::NameID::get_id()
{
	return _id;
}

void ecore::NameID::operator++()
{
	++_id;
}

void ecore::NameID::operator--()
{
	if (_id > 0)
		--_id;
}

ecore::NameID& ecore::NameID::operator=(const NameID& other)
{
	this->_id = other._id;
	return *this;
}

ecore::NameID::operator uint32_t()
{
	return _id;
}

std::map<ecore::ObjectName, ecore::NameID> ecore::ObjectName::_nameTable;

ecore::ObjectName::ObjectName(std::string newName)
{
	_name = new char[MAX_NAME_LENGTH];
	if (newName.length() > MAX_NAME_LENGTH)
	{
		LOG_ERROR("ObjectName::ObjectName(): New name is too long")
		strcpy(_name, "NoName");
		return;
	}
	
	strcpy(_name, newName.c_str());
	LOG_INFO("_name: {}", std::string(_name).c_str())
	auto it = _nameTable.find(*this);
	if (it != _nameTable.end())
	{
		++it->second;
		_nameID = it->second;
	}
	else
	{
		_nameTable[*this] = NameID();
	}
}

ecore::ObjectName::~ObjectName()
{
	destroy_name();
}

void ecore::ObjectName::change_name(std::string newName)
{
	if (newName.length() > 128)
	{
		LOG_ERROR("ObjectName::change_name(): New name is too long")
		return;
	}

	delete_name_from_table();
	strcpy(_name, newName.c_str());

	auto it = _nameTable.find(*this);
	if (it != _nameTable.end())
	{
		++it->second;
		_nameID = it->second;
	}
	else
	{
		_nameTable[*this] = NameID();
	}
}

void ecore::ObjectName::destroy_name()
{
	delete_name_from_table();
	delete[] _name;
}

std::string ecore::ObjectName::get_name()
{
	std::string name(_name);
	if (_nameID.get_id() > 0)
		name += "_" + std::to_string(_nameID);
	return name;
}

ecore::ObjectName& ecore::ObjectName::operator=(const ObjectName& other)
{
	if (*this == other)
		return *this;
	delete_name_from_table();
	auto it = _nameTable.find(other);

	++it->second;
	_nameID = it->second;
	strcpy(this->_name, other._name);

	return *this;
}

bool ecore::ObjectName::operator==(const ObjectName& name) const
{
	return strcmp(this->_name, name._name) == 0;
}

bool ecore::ObjectName::operator<(const ObjectName& name) const
{
	return strcmp(this->_name, name._name) < 0;
}

// Have to make improvements to handle more situation
void ecore::ObjectName::delete_name_from_table()
{
	auto it = _nameTable.find(*this);
	if (_nameID > 0 && it != _nameTable.end())
	{
		--it->second;
	}
	else if (_nameID == 0 && it != _nameTable.end() && it->second == 0)
	{
		_nameTable.erase(it);
	}
}

void ecore::ObjectName::tests()
{
	ObjectName name1("Wall");
	ObjectName name2("Wall");
	ObjectName name3("Gun1");
	LOG_INFO("Name1 before editing: {}", name1.get_name().c_str())
	LOG_INFO("Name2 before editing: {}", name2.get_name().c_str())
	LOG_INFO("Name3 before editing: {}", name3.get_name().c_str())

	name1 = name3;
	LOG_INFO("Name1 after the first editing: {}", name1.get_name().c_str())
	name2.change_name("Gun1");
	LOG_INFO("Name2 after the first editing: {}", name2.get_name().c_str())
	name1.change_name("Wall");
	LOG_INFO("Name 1 after the second editind: {}", name1.get_name())
}
