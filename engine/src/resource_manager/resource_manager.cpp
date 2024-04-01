#include "resource_manager.h"
#include "resource_events.h"
#include "engine_core/material/materials.h"
#include "engine_core/material/shader.h"
#include "profiler/logger.h"
#include "utils.h"

#include <lz4/lz4.h>

using namespace ad_astris;
using namespace resource;

ResourceManager::ResourceManager(io::FileSystem* fileSystem, events::EventManager* eventManager)
	: _fileSystem(fileSystem), _eventManager(eventManager)
{
	_resourceConverter = ResourceConverter(_fileSystem);
	LOG_INFO("ResourceManager::ResourceManager(): Initialized resource converter")
	_resourcePool = ResourcePool();
	LOG_INFO("ResourceManager::ResourceManager(): Initialized resource pool")
	_resourceDataTable = ResourceDataTable(_fileSystem, &_resourcePool);
	_resourceDataTable.load_table(_builtinResourcesContext);
	LOG_INFO("ResourceManager::ResourceManager(): Initialized resource table")
	// load_builtin_resources();
	// LOG_INFO("ResourceManager::ResourceManager(): Loaded builtin resources")
}

ResourceManager::~ResourceManager()
{
	_resourcePool.cleanup();
}

ResourceAccessor<ecore::Level> ResourceManager::create_level(io::URI& path)
{
	std::string strLevelName = io::Utils::get_file_name(path);
	ecore::ObjectName* levelName = _resourcePool.allocate<ecore::ObjectName>(strLevelName.c_str());
	io::File* levelFile = _resourcePool.allocate<ResourceFile>(path);
	ecore::Level* level = _resourcePool.allocate<ecore::Level>(path, levelName);
	level->serialize(levelFile);

	ResourceData resourceData;
	resourceData.file = levelFile;
	resourceData.object = level;
	resourceData.metadata.path = path;
	resourceData.metadata.type = ResourceType::LEVEL;
	resourceData.metadata.objectName = levelName;

	_resourceDataTable.add_resource(&resourceData);
	
	return level;
}

ResourceAccessor<ecore::Level> ResourceManager::load_level(io::URI& path)
{
	UUID uuidByName = _resourceDataTable.get_uuid_by_name(path);
	ResourceData* resourceData = _resourceDataTable.get_resource_data(uuidByName);
	io::File* file = resourceData->file;
	ecore::Level* level = static_cast<ecore::Level*>(resourceData->object);

	if (file)
		return level;
	
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	file = _resourcePool.allocate<ResourceFile>(path);
	file->deserialize(data, size);
	_fileSystem->unmap_after_reading(data);
	
	level = _resourcePool.allocate<ecore::Level>();
	level->deserialize(file, resourceData->metadata.objectName);

	resourceData->file = file;
	resourceData->object = level;
	
	return level;
}

void ResourceManager::load_builtin_resources()
{
	_builtinResourcesContext.clear();
}

template <typename T>
ResourceAccessor<T> ResourceManager::create_new_resource(FirstCreationContext<T>& creationContext)
{
	
}

template<>
ResourceAccessor<ecore::OpaquePBRMaterial> ResourceManager::create_new_resource(
	FirstCreationContext<ecore::OpaquePBRMaterial>& creationContext)
{
	ecore::ObjectName* objectName = _resourcePool.allocate<ecore::ObjectName>(creationContext.materialName.c_str());
	
	io::URI absoluteMaterialPath = creationContext.materialPath + "/" + objectName->get_full_name().c_str() + ".aares";
	if (io::Utils::is_relative(absoluteMaterialPath))
		absoluteMaterialPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), absoluteMaterialPath);

	ResourceData resData{};
	resData.metadata.path = absoluteMaterialPath;
	resData.metadata.type = ResourceType::MATERIAL;
	resData.metadata.objectName = objectName;
	resData.object = _resourcePool.allocate<ecore::OpaquePBRMaterial>(creationContext.materialSettings, objectName);
	resData.object->set_path(absoluteMaterialPath);
	resData.file = _resourcePool.allocate<ResourceFile>(absoluteMaterialPath);
	resData.object->make_dirty();
	
	_resourceDataTable.add_resource(&resData);

	OpaquePBRMaterialCreatedEvent event(static_cast<ecore::OpaquePBRMaterial*>(resData.object));
	_eventManager->enqueue_event(event);

	return resData.object;
}

template<>
ResourceAccessor<ecore::TransparentMaterial> ResourceManager::create_new_resource(
	FirstCreationContext<ecore::TransparentMaterial>& creationContext)
{
	ecore::ObjectName* objectName = _resourcePool.allocate<ecore::ObjectName>(creationContext.materialName.c_str());
	
	io::URI absoluteMaterialPath = creationContext.materialPath + "/" + objectName->get_full_name().c_str() + ".aares";
	if (io::Utils::is_relative(absoluteMaterialPath))
		absoluteMaterialPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), absoluteMaterialPath);
	
	ResourceData resData{};
	resData.metadata.path = absoluteMaterialPath;
	resData.metadata.type = ResourceType::MATERIAL;
	resData.metadata.objectName = objectName;
	resData.object = _resourcePool.allocate<ecore::TransparentMaterial>(creationContext.materialSettings, objectName);
	resData.file = _resourcePool.allocate<resource::ResourceFile>(absoluteMaterialPath);
	resData.object->make_dirty();

	_resourceDataTable.add_resource(&resData);

	return resData.object;
}

void ResourceManager::save_resources()
{
	_resourceDataTable.save_table();
	LOG_INFO("ResourceMangaer::save_resources(): Saved resource table")
	_resourceDataTable.save_resources();
	LOG_INFO("ResourceManager::save_resources(): Saved resources")
}

void ResourceManager::destroy_resource(UUID uuid)
{
	_resourceDataTable.destroy_resource(uuid);
}

/** @warning MEMORY LEAK, uint8_t* data, should be tested if everything works correct after delete[]
 * 
 */
void ResourceManager::write_to_disk(io::File* file)
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

io::File* ResourceManager::read_from_disk(io::URI& path, bool isShader)
{
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::File* file = _resourcePool.allocate<resource::ResourceFile>(path);
	
	if (isShader)
		file->set_binary_blob(data, size);
	else
		file->deserialize(data, size);
	
	_fileSystem->unmap_after_reading(data);
	return file;
}

template <typename T>
void ResourceManager::send_resource_loaded_event(T* resourceObject)
{
	
}

template<>
void ResourceManager::send_resource_loaded_event(ecore::StaticModel* resourceObject)
{
	StaticModelLoadedInEngineEvent event(resourceObject);
	_eventManager->enqueue_event(event);
}

template<>
void ResourceManager::send_resource_loaded_event(ecore::Texture2D* resourceObject)
{
	Texture2DLoadedEvent event(resourceObject);
	_eventManager->enqueue_event(event);
}

template<>
void ResourceManager::send_resource_loaded_event(ecore::OpaquePBRMaterial* materialObject)
{
	
}

template <typename T>
void ResourceManager::send_resource_created_event(T* resourceObject)
{
	
}

template<>
void ResourceManager::send_resource_created_event(ecore::Texture2D* resourceObject)
{
	Texture2DCreatedEvent event(resourceObject);
	_eventManager->enqueue_event(event);
}

template<>
void ResourceManager::send_resource_created_event(ecore::StaticModel* resourceObject)
{
	StaticModelFirstCreationEvent event(resourceObject);
	_eventManager->enqueue_event(event);
}

void BuiltinResourcesContext::clear()
{
	materialTemplateNames.clear();
}
