#include "resource_data_table.h"
#include "file_system/utils.h"
#include "utils.h"

#include <lz4/lz4.h>

#include <execution>
#include <future>

#include "resource_manager.h"

using namespace ad_astris;

resource::ResourceDataTable::ResourceDataTable(io::FileSystem* fileSystem, ResourcePool* resourcePool)
	: _fileSystem(fileSystem), _resourcePool(resourcePool), _resourceDeleterVisitor(resourcePool)
{

}

resource::ResourceDataTable::~ResourceDataTable()
{
	_uuidToResourceData.clear();
}

void resource::ResourceDataTable::load_table(BuiltinResourcesContext& context)
{
	io::URI path = _fileSystem->get_project_root_path() + "/configs/resource_table.ini";
	_config.load_from_file(path);

	for (auto section : _config)
	{
		UUID uuid = section.get_option_value<uint64_t>("UUID");
		if (check_uuid_in_table(uuid))
			continue;
		uint32_t nameID = section.get_option_value<uint64_t>("NameID");
		std::string name = section.get_option_value<std::string>("Name");

		bool builtin = section.get_option_value<bool>("Builtin");
		ResourceData resData{};
		if (builtin)
		{
			resData.metadata.path = io::Utils::get_absolute_path_to_file(_fileSystem->get_engine_root_path(), section.get_name().c_str());
		}
		else
		{
			resData.metadata.path = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), section.get_name().c_str());
		}
		resData.metadata.type = Utils::get_enum_resource_type(section.get_option_value<std::string>("Type"));
		resData.metadata.objectName = _resourcePool->allocate<ecore::ObjectName>(name.c_str(), nameID);
		resData.metadata.builtin = builtin;
		if (resData.metadata.type == ResourceType::MATERIAL_TEMPLATE)
			context.materialTemplateNames.push_back(uuid);
		_uuidToResourceData[uuid] = resData;
		_nameToUUID[resData.metadata.objectName->get_full_name()] = uuid;
	}
	_config.unload();
}

void resource::ResourceDataTable::save_table()
{
	for (auto& data : _uuidToResourceData)
	{
		ResourceData& resData = data.second;
		
		io::URI relativePath;
		if (resData.metadata.builtin)
		{
			relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_engine_root_path(), resData.metadata.path);
		}
		else
		{
			relativePath = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), resData.metadata.path);
		}
			
		io::Utils::replace_back_slash_to_forward(relativePath);
		Section newSection(relativePath.c_str());
		
		newSection.set_option("UUID", (uint64_t)data.first);
		newSection.set_option("Type", Utils::get_str_resource_type(resData.metadata.type));
		newSection.set_option("Builtin", resData.metadata.builtin);
		newSection.set_option("Name", resData.metadata.objectName->get_name_without_id());
		newSection.set_option("NameID", (uint64_t)resData.metadata.objectName->get_name_id());
		
		_config.set_section(newSection);
	}

	_config.save(_fileSystem);
}

// TODO CHECK IF OBJECT IS NULLPTR TO CONTINUE ON THE NEXT ITERATION
void resource::ResourceDataTable::save_resources()
{
	for (auto& pair : _uuidToResourceData)
	{
		ResourceData& resourceData = pair.second;

		if (resourceData.metadata.type == ResourceType::SHADER || !resourceData.object)
			continue;
		
		io::File* file = resourceData.file;
		ecore::Object* object = resourceData.object;
		ResourceMetadata& metadata = resourceData.metadata;

		uint8_t* data{ nullptr };
		uint64_t size = 0;
		object->serialize(file);
		file->serialize(data, size);

		io::Stream* stream = _fileSystem->open(file->get_file_path(), "wb");
		stream->write(data, sizeof(uint8_t), size);
		_fileSystem->close(stream);
		delete[] data;
	}
}

bool resource::ResourceDataTable::check_resource_in_table(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && it->second.object)
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_table(io::URI& path)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	if (it != _nameToUUID.end())
		return true;
	return false;
}

bool resource::ResourceDataTable::check_name_in_table(const std::string& name)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _nameToUUID.find(name);
	if (it != _nameToUUID.end())
		return true;
	return false;
}

bool resource::ResourceDataTable::check_uuid_in_table(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end())
		return true;
	return false;
}

UUID resource::ResourceDataTable::get_uuid_by_name(io::URI& path)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	std::string name = io::Utils::get_file_name(path);
	auto it = _nameToUUID.find(name);
	return it->second;
}

UUID resource::ResourceDataTable::get_uuid_by_name(const std::string& name)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _nameToUUID.find(name);
	return it->second;
}

void resource::ResourceDataTable::add_resource(ResourceData* resource)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	UUID uuid = resource->object->get_uuid();
	auto it = _uuidToResourceData.find(uuid);
	if (it != _uuidToResourceData.end() && resource)
	{
		if (it->second.object)
			it->second.object->accept(_resourceDeleterVisitor);
		if (it->second.file)
			it->second.file->accept(_resourceDeleterVisitor);
		if (it->second.metadata.objectName)
			_resourcePool->free(it->second.metadata.objectName);
		
		it->second = *resource;
		//io::URI path = resource->file->get_file_path();
		//_nameToUUID[io::Utils::get_file_name(path)] = it->first;
	}
	else if (it == _uuidToResourceData.end() && resource)
	{
		_uuidToResourceData[uuid] = *resource;
		_nameToUUID[resource->object->get_name()->get_full_name()] = uuid;
	}
}

void resource::ResourceDataTable::add_empty_resource(ResourceData* resource, UUID uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	if (it == _uuidToResourceData.end())
	{
		_uuidToResourceData[uuid] = *resource;
		_nameToUUID[resource->metadata.objectName->get_full_name()] = uuid;
	}
	else
	{
		LOG_ERROR("ResourceDataTable::add_empty_resource(): Can't add an empty resource with UUID {} because resource with this UUID was created earlier", (uint64_t)uuid)
	}
}

void resource::ResourceDataTable::destroy_resource(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	if (it == _uuidToResourceData.end())
	{
		LOG_ERROR("ResourceDataTable::destroy_resource(): Invalid UUID")
		return;
	}

	if (io::Utils::exists(_fileSystem, it->second.object->get_path()))
		remove(it->second.object->get_path().c_str());
	
	ResourceData resourceData = it->second;
	std::string name = it->second.metadata.objectName->get_full_name();
	_nameToUUID.erase(_nameToUUID.find(name));
	resourceData.object->accept(_resourceDeleterVisitor);
	_resourcePool->free(resourceData.metadata.objectName);
	
	_uuidToResourceData.erase(it);
	LOG_INFO("ResourceDataTable::destroy_resource(): Destroyed resource {}", name)
}

// No if statement to check resource state cause it's implemented in ResourceManager where I use func
// check_resource_in_cache() and make async loading
io::File* resource::ResourceDataTable::get_resource_file(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	return it->second.file;
}

ecore::Object* resource::ResourceDataTable::get_resource_object(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	return it->second.object;
}

resource::ResourceData* resource::ResourceDataTable::get_resource_data(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	return &_uuidToResourceData.find(uuid)->second;
}

resource::ResourceType resource::ResourceDataTable::get_resource_type(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	return it->second.metadata.type;
}

io::URI resource::ResourceDataTable::get_resource_path(UUID& uuid)
{
	std::scoped_lock<std::mutex> lock(_mutex);
	auto it = _uuidToResourceData.find(uuid);
	return it->second.metadata.path;
}
