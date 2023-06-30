#include "resource_data_table.h"
#include "file_system/utils.h"
#include "utils.h"

#include <lz4.h>

#include <algorithm>
#include <execution>
#include <future>

using namespace ad_astris;

resource::ResourceDataTable::ResourceDataTable(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	
}

resource::ResourceDataTable::~ResourceDataTable()
{
	for (auto& res : _uuidToResourceData)
	{
		delete res.second.file;
		delete res.second.object;
		delete res.second.metadata.objectName;
	}
}

void resource::ResourceDataTable::load_table()
{
	io::URI path = io::Utils::get_absolute_path_to_file(_fileSystem, "configs/resource_table.ini");
	_config.load_from_file(path);

	for (auto section : _config)
	{
		UUID uuid = section.get_option_value<uint64_t>("UUID");
		uint32_t nameID = section.get_option_value<uint64_t>("NameID");
		std::string name = section.get_option_value<std::string>("Name");
		ResourceData resData{};
		resData.metadata.path = section.get_name().c_str();
		resData.metadata.type = Utils::get_enum_resource_type(section.get_option_value<std::string>("Type"));
		resData.metadata.objectName = new ecore::ObjectName(name.c_str(), nameID);

		_uuidToResourceData[uuid] = resData;
	}

	_config.unload();
}

void resource::ResourceDataTable::save_table()
{
	for (auto& data : _uuidToResourceData)
	{
		ResourceData& resData = data.second;
		io::URI path = resData.metadata.path;
		io::Utils::replace_back_slash_to_forward(path);

		Section newSection(path.c_str());
		newSection.set_option("UUID", data.first);

		if (!resData.object)
		{
			newSection.set_option("Type", Utils::get_str_resource_type(resData.metadata.type));
			newSection.set_option("Name", resData.metadata.objectName->get_name_without_id());
			newSection.set_option("NameID", (uint64_t)resData.metadata.objectName->get_name_id());
		}
		else
		{
			newSection.set_option("Type", resData.object->get_type());
			ecore::ObjectName* name = resData.object->get_name();
			newSection.set_option("Name", name->get_name_without_id());
			newSection.set_option("NameID", (uint64_t)name->get_name_id());
		}
		
		_config.set_section(newSection);
	}
	
	_config.save(_fileSystem);
}

// TODO CHECK IF OBJECT IS NULLPTR TO CONTINUE ON THE NEXT ITERATION
void resource::ResourceDataTable::save_resources()
{
	for (auto& pair : _uuidToResourceData)
	{
		LOG_INFO("Start saving")
		ResourceData& resourceData = pair.second;
		io::IFile* file = resourceData.file;
		ecore::Object* object = resourceData.object;

		uint8_t* data{ nullptr };
		uint64_t size = 0;
		LOG_INFO("Before object serialization")
		object->serialize(file);
		LOG_INFO("File metadata: {}", file->get_metadata())
		LOG_INFO("Before file serialization")
		file->serialize(data, size);

		LOG_INFO("Before openging stream")
		io::Stream* stream = _fileSystem->open(file->get_file_path(), "wb");
		LOG_INFO("Before writing")
		stream->write(data, sizeof(uint8_t), size);
		LOG_INFO("Before closing")
		_fileSystem->close(stream);
		LOG_INFO("Before deleting")
		delete[] data;
		LOG_INFO("Finish saving")
	}
}

bool resource::ResourceDataTable::check_resource_in_table(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && it->second.object)
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_table(io::URI& path)
{
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	if (it != _nameToUUID.end())
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_table(ecore::ObjectName& name)
{
	// TODO maybe I should change name.get_string() to name.get_string_without_id()
	auto it = _nameToUUID.find(name.get_string());
	if (it != _nameToUUID.end())
		return true;
	return false;
}

bool resource::ResourceDataTable::check_uuid_in_table(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end())
		return true;
	return false;
}

UUID resource::ResourceDataTable::get_uuid_by_name(io::URI& path)
{
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	return it->second;
}

UUID resource::ResourceDataTable::get_uuid_by_name(ecore::ObjectName& name)
{
	auto it = _nameToUUID.find(name.get_string());
	return it->second;
}

void resource::ResourceDataTable::add_resource(ResourceData* resource)
{
	UUID uuid = resource->object->get_uuid();
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && resource)
	{
		if (it->second.object)
			delete it->second.object;
		if (it->second.file)
			delete it->second.file;
		
		it->second = *resource;
		io::URI path = resource->file->get_file_path();
		_nameToUUID[io::Utils::get_file_name(path)] = it->first;
	}
	else if (it == _uuidToResourceData.end() && resource)
	{
		_uuidToResourceData[uuid] = *resource; 
	}
}

void resource::ResourceDataTable::destroy_resource(UUID& uuid)
{
	// Should test it
	auto it = _uuidToResourceData.find(uuid);
	if (it == _uuidToResourceData.end())
	{
		LOG_ERROR("ResourceDataTable::destroy_resource(): Invalid UUID")
		return;
	}
	
	ResourceData resourceData = it->second;
	std::string name = io::Utils::get_file_name(resourceData.file->get_file_path());
	_nameToUUID.erase(_nameToUUID.find(name));
	delete resourceData.file;
	delete resourceData.object;
	_uuidToResourceData.erase(it);
}

// No if statement to check resource state cause it's implemented in ResourceManager where I use func
// check_resource_in_cache() and make async loading
io::IFile* resource::ResourceDataTable::get_resource_file(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.file;
}

ecore::Object* resource::ResourceDataTable::get_resource_object(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.object;
}

resource::ResourceData* resource::ResourceDataTable::get_resource_data(UUID& uuid)
{
	return &_uuidToResourceData.find(uuid)->second;
}

io::URI resource::ResourceDataTable::get_path(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.metadata.path;
}