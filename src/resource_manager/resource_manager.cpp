#include "profiler/logger.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "resource_manager.h"
#include "utils.h"
#include "file_system/utils.h"

#include <lz4.h>

#include <algorithm>

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
	io::Stream* stream = _fileSystem->open("configs/resource_table.aarestable", "rb");
	uint64_t strSize;
	stream->read(&strSize, sizeof(uint64_t), 1);
	std::string strTable;
	strTable.resize(strSize);
	stream->read(&strTable, sizeof(char), strSize);
	_fileSystem->close(stream);

	nlohmann::json table = nlohmann::json::parse(strTable);
	std::vector<std::string> tableData;
	tableData.resize(2);
	
	for (auto& item : table.items())
	{
		tableData = table[item.key().c_str()].get<std::vector<std::string>>();
		UUID uuid((uint64_t)tableData[0].c_str());
		ResourceData resourceData{};
		io::IFile* file = new io::ResourceFile(item.key().c_str());
		resourceData.file = file;
		//resourceData.type = Utils::get_enum_resource_type(tableData[1]);
		_uuidToResourceData[uuid] = resourceData;
		_nameToUUID[file->get_file_name()] = uuid;
	}
}

void resource::ResourceDataTable::save_table()
{
	nlohmann::json table;
	std::vector<std::string> tableData;
	tableData.resize(2);
	for (auto& data : _uuidToResourceData)
	{
		tableData[0] = std::to_string(data.first);
		tableData[1] = data.second.object->get_type();
		table[data.second.object->get_path().c_str()] = (uint64_t)data.first;
	}

	io::Stream* stream = _fileSystem->open("configs/resource_table.aarestable", "wb");
	std::string strTable = table.dump();
	uint64_t size = strTable.size();
	stream->write(&size, sizeof(uint64_t), 1);
	stream->write(strTable.data(), sizeof(char), size);
	_fileSystem->close(stream);
}

bool resource::ResourceDataTable::check_resource_in_cache(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && it->second.object)
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_cache(io::URI& path)
{
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	if (it != _nameToUUID.end())
		return true;
	return false;
}

UUID resource::ResourceDataTable::get_uuid_by_name(io::URI& path)
{
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	return it->second;
}

void resource::ResourceDataTable::add_resource(ResourceData* resource)
{
	UUID uuid = resource->object->get_uuid();
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && resource)
	{
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

io::URI resource::ResourceDataTable::get_path(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.file->get_file_path();
}

resource::ResourceType resource::ResourceDataTable::get_resource_type(UUID& uuid)
{
	//return _uuidToResourceData[uuid].type;
	return ResourceType::UNDEFINED;
}

resource::ResourceManager::ResourceManager(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_resourceConverter = ResourceConverter(_fileSystem);
	_resourceDataTable = new ResourceDataTable(_fileSystem);
}

resource::ResourceManager::~ResourceManager()
{
	delete _resourceDataTable;
}

void resource::ResourceManager::write_to_disk(io::IFile* file, io::URI& originalPath)
{
	io::URI path = file->get_file_path();		// temporary solution

	uint8_t* data{ nullptr };
	uint64_t size = 0;
	file->serialize(data, size);
	
	io::Stream* stream = _fileSystem->open(path, "wb");
	stream->write(data, sizeof(uint8_t), size);
	_fileSystem->close(stream);
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
