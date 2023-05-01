#include "object_name.h"
#include "profiler/logger.h"

#include <cstring>
#include <algorithm>

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

bool ecore::NameID::operator==(const NameID& other)
{
	return this->_id == other._id;
}

ecore::NameID::operator uint32_t()
{
	return _id;
}

ecore::NameIDTable::NameIDTable()
{
	_table.push_back(NameID());
}

ecore::NameID ecore::NameIDTable::get_next_id()
{
	if (_removedNameIDs.empty())
	{
		NameID id = _table.back();
		++id;
		_table.push_back(id);
		return _table.back();
	}
	
	NameID id = _removedNameIDs.front();
	_table.push_back(id);
	_removedNameIDs.erase(_removedNameIDs.begin());
	std::sort(_table.begin(), _table.end());
	return id;
}

void ecore::NameIDTable::remove_id(NameID nameId)
{
	_table.erase(std::find(_table.begin(), _table.end(), nameId));
	_removedNameIDs.push_back(nameId);
	std::sort(_removedNameIDs.begin(), _removedNameIDs.end());
}

bool ecore::NameIDTable::is_empty()
{
	return _table.empty();
}

std::map<ecore::ObjectName, ecore::NameIDTable> ecore::ObjectName::_nameTable;

ecore::ObjectName::ObjectName(const char* newName)
{
	_name = new char[MAX_NAME_LENGTH];
	if (strlen(newName) > MAX_NAME_LENGTH)
	{
		LOG_ERROR("ObjectName::ObjectName(): New name is too long")
		strcpy(_name, "NoName");
		return;
	}
	
	strcpy(_name, newName);
	auto it = _nameTable.find(*this);
	if (it != _nameTable.end())
	{
		_nameID = it->second.get_next_id();
	}
	else
	{
		add_new_name_to_table();
		_nameID = NameID();
	}
}

ecore::ObjectName::~ObjectName()
{
	if (_name)
		delete[] _name;
}

void ecore::ObjectName::change_name(const char* newName)
{
	if (strlen(newName) > 128)
	{
		LOG_ERROR("ObjectName::change_name(): New name is too long")
		return;
	}

	if (!_name)
	{
		_name = new char[MAX_NAME_LENGTH];
	}
	else
	{
		delete_name_from_table();
	}
	
	strcpy(_name, newName);

	auto it = _nameTable.find(*this);
	if (it != _nameTable.end())
	{
		_nameID = it->second.get_next_id();
	}
	else
	{
		add_new_name_to_table();
		_nameID = NameID();
	}
}

void ecore::ObjectName::change_name(ObjectName& otherName)
{
	if (!_name)
	{
		_name = new char[MAX_NAME_LENGTH];
	}
	else
	{
		delete_name_from_table();
	}

	strcpy(_name, otherName._name);
	_nameID = _nameTable.find(otherName)->second.get_next_id();
}

void ecore::ObjectName::destroy_name()
{
	delete_name_from_table();
	delete[] _name;
	_name = nullptr;
}

std::string ecore::ObjectName::get_string()
{
	std::string name(_name);
	if (_nameID.get_id() > 0)
		name += "_" + std::to_string(_nameID);
	return name;
}

std::string ecore::ObjectName::get_name_without_instance()
{
	return std::string{_name};
}

void ecore::ObjectName::cleanup()
{
	for (auto& it : _nameTable)
	{
		delete[] it.first._name;
	}
}

ecore::ObjectName& ecore::ObjectName::operator=(const ObjectName& otherName)
{
	if (!_name)
		_name = new char[MAX_NAME_LENGTH];
	strcpy(_name, otherName._name);
	this->_nameID = otherName._nameID;
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

	if (it != _nameTable.end())
	{
		NameIDTable table = it->second;
		table.remove_id(_nameID);
		_nameTable[*this] = table;
		if (it->second.is_empty())
		{
			//delete[] it->first._name;
			_nameTable.erase(it);
		}
	}
}

void ecore::ObjectName::add_new_name_to_table()
{
	char* keyName = new char[strlen(_name)];
	strcpy(keyName, this->_name);
	ObjectName objectName{};
	objectName._name = keyName;
	_nameTable[objectName] = NameIDTable();
	objectName._name = nullptr;
}

void ecore::tests()
{
	ObjectName name1("Wall");
	ObjectName name2(name1.get_name_without_instance().c_str());
	ObjectName name3("Gun1");
	LOG_INFO("Name1 before editing: {}", name1.get_string().c_str())
	LOG_INFO("Name2 before editing: {}", name2.get_string().c_str())
	LOG_INFO("Name3 before editing: {}", name3.get_string().c_str())
	
	name1.change_name(name3);
	LOG_INFO("Name1 after the first editing: {}", name1.get_string().c_str())
	name2.change_name("Gun1");
	LOG_INFO("Name2 after the first editing: {}", name2.get_string().c_str())
	name1.change_name("Wall");
	LOG_INFO("Name 1 after the second editind: {}", name1.get_string())
	name2.change_name("Wall");
	LOG_INFO("Name2 after the second editing: {}", name2.get_string().c_str())
	name3.change_name(name2);
	LOG_INFO("Name3 after the first editing: {}", name3.get_string().c_str())
	ObjectName tempName;
	tempName = name3;
	LOG_INFO("Temp name: {}", tempName.get_string())
}
