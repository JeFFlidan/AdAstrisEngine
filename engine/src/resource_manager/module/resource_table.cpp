#include "resource_table.h"
#include "engine_core/model/model.h"
#include "engine_core/texture/texture.h"
#include "engine_core/level/level.h"
#include "resource_manager/resource_events.h"

using namespace ad_astris;
using namespace resource::impl;

ResourceTable::ResourceTable(ResourcePool* resourcePool) : _resourcePool(resourcePool)
{
	setup_resource_vtables();
	load_config();
}

void ResourceTable::save_config()
{
	for (auto& pair : _resourceDescByUUID)
	{
		ResourceDesc& resourceDesc = pair.second;
		
		io::URI relativePath = io::Utils::get_relative_path_to_file(FILE_SYSTEM()->get_project_root_path(), resourceDesc.path);
			
		io::Utils::replace_back_slash_to_forward(relativePath);
		Section newSection(relativePath.c_str());
		
		newSection.set_option("UUID", (uint64_t)pair.first);
		newSection.set_option("Type", Utils::get_str_resource_type(resourceDesc.type));
		newSection.set_option("Name", resourceDesc.resourceName->get_name_without_id());
		newSection.set_option("NameID", (uint64_t)resourceDesc.resourceName->get_name_id());
		
		_config.set_section(newSection);
	}

	_config.save(FILE_SYSTEM());
}

void ResourceTable::save_resource(UUID uuid)
{
	ResourceDesc* resourceDesc = get_resource_desc(uuid);
	save_resource_internal(resourceDesc);
}

void ResourceTable::save_resources()
{
	for (auto& pair : _resourceDescByUUID)
	{
		save_resource_internal(&pair.second);
	}
}

void ResourceTable::add_resource(const ResourceDesc& resourceDesc)
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(resourceDesc.resource->get_uuid());
	if (it != _resourceDescByUUID.end())
	{
		if (it->second.resource)
			destroy_resource(it->first);
		_resourceDescByUUID[it->first] = resourceDesc;
	}
	else
	{
		_resourceDescByUUID[resourceDesc.resource->get_uuid()] = resourceDesc;
	}
}

void ResourceTable::unload_resource(UUID uuid)
{
	ResourceDesc* resourceDesc = get_resource_desc(uuid);
	if (resourceDesc && resourceDesc->resource)
	{
		_vtableByResourceType[resourceDesc->type].destroy(resourceDesc->resource);
		resourceDesc->resource = nullptr;
	}
}

void ResourceTable::destroy_resource(UUID uuid)
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(uuid);
	if (it == _resourceDescByUUID.end())
	{
		LOG_ERROR("ResourceTable::destroy_resource(): ResourceTable does not have resource with UUID {}", uuid)
		return;
	}

	ResourceDesc& resourceDesc = it->second;
	if (resourceDesc.resource)
	{
		_vtableByResourceType[resourceDesc.type].destroy(resourceDesc.resource);
	}
	remove(resourceDesc.path.c_str());
	std::string name = resourceDesc.resourceName->get_full_name();
	_uuidByName.erase(_uuidByName.find(name));
	_resourceDescByUUID.erase(it);
	LOG_INFO("ResourceTable::destroy_resource(): Destroyed resource {}", name)
}

bool ResourceTable::is_resource_loaded(UUID uuid) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(uuid);
	if (it == _resourceDescByUUID.end() || it->second.resource == nullptr)
		return false;
	return true;
}

bool ResourceTable::is_resource_loaded(const std::string& name) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _uuidByName.find(name);
	if (it == _uuidByName.end())
		return false;
	auto it2 = _resourceDescByUUID.find(it->second);
	if (it2 == _resourceDescByUUID.end() || it2->second.resource == nullptr)
		return false;
	return true;
}

bool ResourceTable::is_resource_desc_valid(UUID uuid) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(uuid);
	if (it == _resourceDescByUUID.end())
		return false;
	return true;
}

bool ResourceTable::is_resource_desc_valid(const std::string& name) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _uuidByName.find(name);
	if (it == _uuidByName.end())
		return false;
	auto it2 = _resourceDescByUUID.find(it->second);
	if (it2 == _resourceDescByUUID.end())
		return false;
	return true;
}

bool ResourceTable::is_uuid_valid(UUID uuid) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(uuid);
	if (it == _resourceDescByUUID.end())
		return false;
	return true;
}

UUID ResourceTable::get_resource_uuid(const std::string& name) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _uuidByName.find(name);
	if (it == _uuidByName.end())
	{
		LOG_ERROR("ResourceTable::get_resource_uuid(): ResourceTable does not have name {}", name)
		return UUID();
	}
	return it->second;
}

ResourceDesc* ResourceTable::get_resource_desc(UUID uuid) const
{
	std::scoped_lock<std::mutex> locker(_mutex);
	auto it = _resourceDescByUUID.find(uuid);
	if (it == _resourceDescByUUID.end())
	{
		LOG_ERROR("ResourceTable::get_resource_desc(): UUID is invalid")
		return nullptr;
	}
	return &const_cast<ResourceDesc&>(it->second);
}

resource::ResourceType ResourceTable::get_resource_type(UUID uuid) const
{
	return get_resource_desc(uuid)->type;
}

ecore::ObjectName* ResourceTable::get_resource_name(UUID uuid) const
{
	return get_resource_desc(uuid)->resourceName;
}

void ResourceTable::setup_resource_vtables()
{
	_vtableByResourceType[ResourceType::MODEL] = { [this](ecore::Object* resource)
	{
		_resourcePool->free(static_cast<ecore::Model*>(resource));
	} };

	_vtableByResourceType[ResourceType::TEXTURE] = { [this](ecore::Object* resource)
	{
		_resourcePool->free(static_cast<ecore::Texture*>(resource));
	} };

	_vtableByResourceType[ResourceType::LEVEL] = { [this](ecore::Object* resource)
	{
		_resourcePool->free(static_cast<ecore::Level*>(resource));
	} };
}

void ResourceTable::load_config()
{
	io::URI path = FILE_SYSTEM()->get_project_root_path() + "/configs/resource_table.ini";
	_config.load_from_file(path);

	for (auto section : _config)
	{
		UUID uuid = section.get_option_value<uint64_t>("UUID");
		if (is_uuid_valid(uuid))
			continue;
		uint32_t nameID = section.get_option_value<uint64_t>("NameID");
		std::string name = section.get_option_value<std::string>("Name");
		
		ResourceDesc resourceDesc{};
		resourceDesc.path = io::Utils::get_absolute_path_to_file(FILE_SYSTEM()->get_project_root_path(), section.get_name().c_str());
		resourceDesc.type = Utils::get_enum_resource_type(section.get_option_value<std::string>("Type"));
		resourceDesc.resourceName = _resourcePool->allocate<ecore::ObjectName>(name.c_str(), nameID);
		_resourceDescByUUID[uuid] = resourceDesc;
		_uuidByName[resourceDesc.resourceName->get_full_name()] = uuid;
	}
	_config.unload();
}

void ResourceTable::save_resource_internal(ResourceDesc* resourceDesc)
{
	if (!resourceDesc->resource && !resourceDesc->resource->is_dirty())
		return;
	io::File file;
	ecore::Object* resource = resourceDesc->resource;
	resource->serialize(&file);

	uint8_t* outputData{ nullptr };
	uint64_t outputDataSize = 0;
	file.serialize(&outputData, &outputDataSize);
	io::Utils::write_file(FILE_SYSTEM(), resourceDesc->path, outputData, outputDataSize);
	delete[] outputData;
}
