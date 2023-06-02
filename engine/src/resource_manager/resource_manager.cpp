#include "profiler/logger.h"
#include "resource_manager.h"
#include "utils.h"
#include "file_system/utils.h"

#include <lz4.h>
#include <inicpp.h>

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
	}
}

void resource::ResourceDataTable::load_table()
{
	io::URI path = io::Utils::get_absolute_path_to_file(_fileSystem, "configs/resource_table.ini");
	inicpp::config tableConfig = inicpp::parser::load_file(path.c_str());
	for (auto& section : tableConfig)
	{
		ResourceData resData{};
		resData.metadata.path = section.get_name().c_str();
		UUID uuid;
		uint32_t nameID;
		std::string name;
		for (auto& opt : section)
		{
			if (opt.get_name() == "UUID")
			{
				uuid = opt.get<inicpp::unsigned_ini_t>();
			}
			else if (opt.get_name() == "Name")
			{
				name = opt.get<inicpp::string_ini_t>();
			}
			else if (opt.get_name() == "NameID")
			{
				nameID = opt.get<inicpp::unsigned_ini_t>();
			}
			else if (opt.get_name() == "Type")
			{
				resData.metadata.type = Utils::get_enum_resource_type(opt.get<inicpp::string_ini_t>());
			}
		}

		resData.metadata.objectName = new ecore::ObjectName(name.c_str(), ecore::NameID(nameID));
		_uuidToResourceData[uuid] = resData;
		_nameToUUID[resData.metadata.objectName->get_string()] = uuid;
	}
}

void resource::ResourceDataTable::save_table()
{
	inicpp::config config{ };
	for (auto& data : _uuidToResourceData)
	{
		ResourceData& resData = data.second;
		io::URI path = resData.metadata.path;
		io::Utils::replace_back_slash_to_forward(path);
		inicpp::section newSection(path.c_str());
		
		inicpp::option uuidOption("UUID");
		uuidOption = data.first;	
		inicpp::option typeOption("Type");
		inicpp::option nameOption("Name");
		inicpp::option nameIdOption("NameID");

		if (!resData.object)
		{
			typeOption = Utils::get_str_resource_type(resData.metadata.type);
			nameOption = resData.metadata.objectName->get_name_without_id();
			nameIdOption = (inicpp::unsigned_ini_t)resData.metadata.objectName->get_name_id();
		}
		else
		{
			typeOption = resData.object->get_type();
			ecore::ObjectName* name = resData.object->get_name();
			nameOption = name->get_name_without_id();
			nameIdOption =  (inicpp::unsigned_ini_t)name->get_name_id();
		}
		
		newSection.add_option(uuidOption);
		newSection.add_option(typeOption);
		newSection.add_option(nameOption);
		newSection.add_option(nameIdOption);
		
		config.add_section(newSection);
	}

	io::URI path = io::Utils::get_absolute_path_to_file(_fileSystem, "configs/resource_table.ini");
	inicpp::parser::save(config, path.c_str());
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

resource::ResourceManager::ResourceManager(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_resourceConverter = ResourceConverter(_fileSystem);
	_resourceDataTable = new ResourceDataTable(_fileSystem);
	_resourceDataTable->load_table();
}

resource::ResourceManager::~ResourceManager()
{
	delete _resourceDataTable;
}

resource::ResourceAccessor<ecore::Level> resource::ResourceManager::load_level(io::URI& path)
{
	// TODO
}

void resource::ResourceManager::save_resources()
{
	_resourceDataTable->save_table();
}

resource::ResourceType resource::ResourceManager::get_resource_type(ecore::ObjectName& objectName)
{
	if (!_resourceDataTable->check_name_in_table(objectName))
	{
		LOG_ERROR("ResourceManager::get_resource_type(): Invalid name {}", objectName.get_string());
		return ResourceType::UNDEFINED;
	}
	UUID uuid = _resourceDataTable->get_uuid_by_name(objectName);
	ResourceData* resData = _resourceDataTable->get_resource_data(uuid);
	return resData->metadata.type;
}

resource::ResourceType resource::ResourceManager::get_resource_type(UUID uuid)
{
	if (!_resourceDataTable->check_uuid_in_table(uuid))
	{
		LOG_ERROR("ResourceManager::get_resource_type(): Invalid UUID {}", uuid)
		return ResourceType::UNDEFINED;
	}
	ResourceData* resData = _resourceDataTable->get_resource_data(uuid);
	return resData->metadata.type;
}

/** @warning MEMORY LEAK, uint8_t* data, should be tested if everything works correct after delete[]
 * 
 */
void resource::ResourceManager::write_to_disk(io::IFile* file, io::URI& originalPath)
{
	io::URI path = file->get_file_path();		// temporary solution

	uint8_t* data{ nullptr };
	uint64_t size = 0;
	file->serialize(data, size);
	
	io::Stream* stream = _fileSystem->open(path, "wb");
	stream->write(data, sizeof(uint8_t), size);
	_fileSystem->close(stream);
	delete[] data;
}

io::IFile* resource::ResourceManager::read_from_disk(io::URI& path)
{
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::IFile* file = new io::ResourceFile(path);
	file->deserialize(data, size);
	_fileSystem->unmap_after_reading(data);
	return file;
}
