#include "profiler/logger.h"
#include "engine_core/model.h"
#include "resource_manager.h"
#include "utils.h"

#include <lz4.h>

#include <algorithm>

using namespace ad_astris;

resource::ResourceDataTable::ResourceDataTable(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	
}

resource::ResourceDataTable::~ResourceDataTable()
{
	for (auto& res : _resources)
	{
		res.vtable->destroy(res.data);
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

	for (auto& item : table.items())
	{
		UUID uuid(item.value());
		ResourceData resourceData{};
		resourceData.pathToResource = io::URI(item.key().c_str());
		_uuidToResourceData[uuid] = resourceData;
		_nameToUUID[utils::get_resource_name(resourceData.pathToResource)] = uuid;
	}
}

void resource::ResourceDataTable::save_table()
{
	nlohmann::json table;
	for (auto& data : _uuidToResourceData)
	{
		table[data.second.pathToResource.c_str()] = (uint64_t)data.first; 
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
	if (it != _uuidToResourceData.end() && it->second.resource)
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_cache(io::URI& path)
{
	std::string name = utils::get_resource_name(path);
	auto it = _nameToUUID.find(name);
	if (it != _nameToUUID.end())
		return true;
	return false;
}

UUID resource::ResourceDataTable::get_uuid_by_name(io::URI& path)
{
	std::string name = utils::get_resource_name(path);
	auto it = _nameToUUID.find(name);
	return it->second;
}

void resource::ResourceDataTable::add_resource_and_path(UUID& uuid, io::URI& path, Resource* resource)
{
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && resource)
	{
		_resources.push_back(*resource);
		it->second.resource = &_resources.back();
		_nameToUUID[utils::get_resource_name(path)] = it->first;
	}
	else if (it == _uuidToResourceData.end() && resource)
	{
		_resources.push_back(*resource);
		ResourceData resourceData;
		resourceData.resource = &_resources.back();
		resourceData.pathToResource = path;
		_uuidToResourceData[uuid] = resourceData; 
	}
	else if (it == _uuidToResourceData.end() && !resource)
	{
		ResourceData resourceData{};
		resourceData.pathToResource = path;
		_uuidToResourceData[uuid] = resourceData;
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
	std::string name = utils::get_resource_name(resourceData.pathToResource);
	_nameToUUID.erase(_nameToUUID.find(name));
	_uuidToResourceData.erase(it);
	
	if (check_resource_in_cache(uuid))
	{
		auto it2 = std::find(_resources.begin(), _resources.end(), *resourceData.resource);
		if (it2 != _resources.end())
		{
			(*it2).vtable->destroy((*it2).data);
			_resources.erase(it2);
		}
	}
}

// No if statement to check resource state cause it's implemented in ResourceManager where I use func
// check_resource_in_cache() and make async loading
resource::Resource* resource::ResourceDataTable::get_resource(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.resource;
}

io::URI& resource::ResourceDataTable::get_path(UUID& uuid)
{
	auto it = _uuidToResourceData.find(uuid);
	return it->second.pathToResource;
}

resource::ResourceManager::ResourceManager(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_resourceConverter = ResourceConverter(_fileSystem);
	_resourceDataTable = new ResourceDataTable(_fileSystem);
	
	ResourceVTable* modelVTable = new ResourceVTable();
	modelVTable->create = ResourceMethods<ecore::Model>::create;
	modelVTable->destroy = ResourceMethods<ecore::Model>::destroy;
	_vtables[ResourceType::MODEL] = modelVTable;
	
	ResourceVTable* levelVTable = new ResourceVTable();
	levelVTable->create = ResourceMethods<ecore::Level>::create;
	levelVTable->destroy = ResourceMethods<ecore::Level>::destroy;
	_vtables[ResourceType::LEVEL] = levelVTable;
}

resource::ResourceManager::~ResourceManager()
{
	delete _resourceDataTable;
	
	for (auto& vtable : _vtables)
		delete vtable.second;
}

void resource::ResourceManager::write_to_disk(ResourceInfo& resourceInfo, io::URI& originalPath)
{
	io::URI path = std::string("assets/" + utils::get_resource_name(originalPath) + ".aares").c_str();		// temporary solution
	
	int compressStaging = LZ4_compressBound(resourceInfo.dataSize);
	LOG_INFO("Compressing staging: {}", compressStaging)
	std::vector<char> binaryBlob;
	binaryBlob.resize(compressStaging);
	uint64_t compressedSize = LZ4_compress_default(
		(const char*)resourceInfo.data,
		binaryBlob.data(),
		resourceInfo.dataSize,
		compressStaging);
	binaryBlob.resize(compressedSize);

	LOG_INFO("Compressed size: {}", compressedSize)
	
	uint64_t metaDataLength = resourceInfo.metaData.size();
	std::string resourceType = utils::get_str_resource_type(resourceInfo.type);
	uint8_t resourceTypeLength = resourceType.size();
	
	io::Stream* stream = _fileSystem->open(path, "wb");

	stream->write(&resourceTypeLength, sizeof(uint8_t), 1);
	stream->write(resourceType.data(), sizeof(char), resourceTypeLength);
	stream->write(&resourceInfo.uuid, sizeof(uint64_t), 1);
	stream->write(&metaDataLength, sizeof(uint64_t), 1);
	stream->write(&compressedSize, sizeof(uint64_t), 1);
	stream->write(&resourceInfo.dataSize, sizeof(uint64_t), 1);
	stream->write(resourceInfo.metaData.data(), sizeof(char), metaDataLength);
	stream->write(binaryBlob.data(), sizeof(uint8_t), compressedSize);
	
	_fileSystem->close(stream);
}

resource::ResourceInfo resource::ResourceManager::read_from_disk(io::URI& path)
{
	io::Stream* stream = _fileSystem->open(path, "rb");
	uint8_t typeLength;
	stream->read(&typeLength, sizeof(uint8_t), 1);
	std::string type;
	type.resize(typeLength);
	stream->read(type.data(), sizeof(char), typeLength);
	uint64_t intUUID;
	stream->read(&intUUID, sizeof(uint64_t), 1);
	uint64_t metaDataLength;
	stream->read(&metaDataLength, sizeof(uint64_t), 1);
	uint64_t compressedBlobSize;
	stream->read(&compressedBlobSize, sizeof(uint64_t), 1);
	uint64_t finalBlobSize;
	stream->read(&finalBlobSize, sizeof(uint64_t), 1);
	std::string metaData;
	metaData.resize(metaDataLength);
	stream->read(metaData.data(), sizeof(char), metaDataLength);

	ResourceInfo resInfo{};
	
	if (finalBlobSize)
	{
		char* compressedBlob = new char[compressedBlobSize];
		stream->read(compressedBlob, sizeof(char), compressedBlobSize);

		char* finalBlob = new char[finalBlobSize];
		LZ4_decompress_safe(compressedBlob, finalBlob, compressedBlobSize, finalBlobSize);

		resInfo.dataSize = finalBlobSize;
		resInfo.data = reinterpret_cast<uint8_t*>(finalBlob);
		delete[] compressedBlob;
	}
	
	resInfo.type = utils::get_enum_resource_type(type);
	resInfo.metaData = metaData;
	resInfo.uuid = UUID(intUUID);
	return resInfo;
}

bool resource::operator==(const Resource& lv, const Resource& rv)
{
	return lv.data == rv.data && lv.vtable == rv.vtable;
}

bool resource::operator==(const ResourceData& lv, const ResourceData& rv)
{
	return lv.resource == rv.resource && lv.pathToResource == rv.pathToResource;
}
