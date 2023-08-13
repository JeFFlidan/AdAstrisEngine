#include "object_name.h"
#include "profiler/logger.h"

#include <cstring>
#include <algorithm>

#define MAX_NAME_LENGTH 128

using namespace ad_astris;

ecore::NameID::NameID(uint32_t newId) : _id(newId)
{
	
}

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

ecore::NameIDTable::NameIDTable(NameID nameId)
{
	_table.push_back(nameId);
	
	if (_table.size() > 1)
	{
		std::sort(_table.begin(), _table.end());
	}
	if (nameId != 0)
	{
		auto it = std::find(_table.begin(), _table.end(), nameId);

		if (_table.size() > 1 && it != _table.begin())
		{
			auto firstIt = it - 1;
			NameID tempId = *firstIt;
			for (; tempId != nameId; ++tempId)
			{
				_removedNameIDs.push_back(tempId);
			}
		}
		else if ((_table.size() > 1 && it == _table.begin()) || _table.empty())
		{
			for (uint32_t i = 0; i != nameId + 1; ++i)
			{
				_removedNameIDs.emplace_back(i);
			}
		}
	}
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

void ecore::NameIDTable::add_id(NameID nameId)
{
	auto tempIt = std::find(_table.begin(), _table.end(), nameId);
	if (tempIt != _table.end())
	{
		LOG_ERROR("NameIDTable::add_id(): NameID {} isn't unique", nameId.get_id())
		return;
	}

	auto it = std::find(_removedNameIDs.begin(), _removedNameIDs.end(), nameId);
	if (it != _removedNameIDs.end())
	{
		_table.push_back(*it);
		_removedNameIDs.erase(it);
		std::sort(_table.begin(), _table.end());
	}
	else
	{
		_table.push_back(nameId);
		std::sort(_table.begin(), _table.end());
		auto it2 = std::find(_table.begin(), _table.end(), nameId);
		if (it2 == _table.begin() && nameId != 0)
		{
			for (uint32_t i = 0; i != nameId; ++i)
			{
				NameID id(i);
				if (std::find(_removedNameIDs.begin(), _removedNameIDs.end(), id) == _removedNameIDs.end())
					_removedNameIDs.emplace_back(i);
			}
		}
		else if (_table.size() > 1)		// TODO HAVE TO THINK ABOUT IT, MAYBE SHOULD REMOVE
		{
			NameID secondId = *(it2 + 1);
			for (uint32_t i = nameId; i != secondId; ++i)
			{
				NameID id(i);
				if (std::find(_removedNameIDs.begin(), _removedNameIDs.end(), id) == _removedNameIDs.end())
					_removedNameIDs.emplace_back(i);
			}
		}
	}
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

std::map<std::string, ecore::NameIDTable> ecore::ObjectName::_nameTable;

ecore::ObjectName::ObjectName(const char* newName)
{
	if (strlen(newName) > MAX_NAME_LENGTH)
	{
		LOG_ERROR("ObjectName::ObjectName(): New name is too long")
		_name = "NoName";
		return;
	}
	
	_name = newName;
	auto it = _nameTable.find(_name);
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

ecore::ObjectName::ObjectName(const char* newName, NameID nameId)
{
	if (!newName)
	{
		LOG_ERROR("ObjectName::ObjectName(): Invalid new name")
		return;
	}

	if (strlen(newName) > MAX_NAME_LENGTH)
	{
		LOG_ERROR("ObjectName::ObjectName(): New name is too long")
		_name = "NoName";
		return;
	}

	_name = newName;
	_nameID = nameId;
	auto it = _nameTable.find(_name);

	if (it == _nameTable.end())
	{
		add_new_name_to_table(_nameID);
	}
	else
	{
		it->second.add_id(_nameID);
	}
}

ecore::ObjectName::~ObjectName()
{
}

void ecore::ObjectName::change_name(const char* newName)
{
	if (strlen(newName) > 128)
	{
		LOG_ERROR("ObjectName::change_name(): New name is too long")
		return;
	}
	
	if (!_name.empty())
	{
		delete_name_from_table();
	}
	
	_name = newName;

	auto it = _nameTable.find(_name);
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
	if (!_name.empty())
	{
		delete_name_from_table();
	}

	_name = otherName._name;
	_nameID = _nameTable.find(otherName._name)->second.get_next_id();
}

void ecore::ObjectName::destroy_name()
{
	delete_name_from_table();
	_name.clear();
}

std::string ecore::ObjectName::get_full_name()
{
	std::string name = _name;
	if (_nameID.get_id() > 0)
		name += "_" + std::to_string(_nameID);
	return name;
}

std::string ecore::ObjectName::get_name_without_id()
{
	return std::string{_name};
}

ecore::NameID ecore::ObjectName::get_name_id()
{
	return _nameID;
}

ecore::ObjectName& ecore::ObjectName::operator=(const ObjectName& otherName)
{
	_name = otherName._name;
	this->_nameID = otherName._nameID;
	return *this;
}

bool ecore::ObjectName::operator==(const ObjectName& name) const
{
	return _name == name._name;
}

bool ecore::ObjectName::operator<(const ObjectName& name) const
{
	return _name < name._name;
}

// Have to make improvements to handle more situation
void ecore::ObjectName::delete_name_from_table()
{
	auto it = _nameTable.find(_name);

	if (it != _nameTable.end())
	{
		NameIDTable table = it->second;
		table.remove_id(_nameID);
		_nameTable[_name] = table;
		if (it->second.is_empty())
		{
			//delete[] it->first._name;
			_nameTable.erase(it);
		}
	}
}

void ecore::ObjectName::add_new_name_to_table(NameID nameID)
{
	_nameTable[_name] = NameIDTable(nameID);
}

void ecore::tests()
{
	ObjectName name1("Wall");
	ObjectName name2(name1.get_name_without_id().c_str());
	ObjectName name3("Gun1");
	LOG_INFO("Name1 before editing: {}", name1.get_full_name().c_str())
	LOG_INFO("Name2 before editing: {}", name2.get_full_name().c_str())
	LOG_INFO("Name3 before editing: {}", name3.get_full_name().c_str())
	
	name1.change_name(name3);
	LOG_INFO("Name1 after the first editing: {}", name1.get_full_name().c_str())
	name2.change_name("Gun1");
	LOG_INFO("Name2 after the first editing: {}", name2.get_full_name().c_str())
	name1.change_name("Wall");
	LOG_INFO("Name 1 after the second editind: {}", name1.get_full_name())
	name2.change_name("Wall");
	LOG_INFO("Name2 after the second editing: {}", name2.get_full_name().c_str())
	name3.change_name(name2);
	LOG_INFO("Name3 after the first editing: {}", name3.get_full_name().c_str())
	ObjectName tempName;
	tempName = name3;
	LOG_INFO("Temp name: {}", tempName.get_full_name())
}
