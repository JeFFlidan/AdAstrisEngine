#include "profiler/logger.h"
#include "resource_manager.h"
#include "utils.h"

#include <lz4.h>
#include <inicpp.h>

using namespace ad_astris;

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

resource::ResourceAccessor<ecore::Level> resource::ResourceManager::create_level(io::URI& path)
{
	io::IFile* levelFile = new io::LevelFile(path);
	ecore::Level* level = new ecore::Level(path);
	level->serialize(levelFile);

	ResourceData resourceData;
	resourceData.file = levelFile;
	resourceData.object = level;
	resourceData.metadata.path = path;
	resourceData.metadata.type = ResourceType::LEVEL;
	// TODO Have to think about ObjectName in metadata. Maybe I need it only when I load resources

	_resourceDataTable->add_resource(&resourceData);

	return level;
}

resource::ResourceAccessor<ecore::Level> resource::ResourceManager::load_level(io::URI& path)
{
	UUID uuidByName = _resourceDataTable->get_uuid_by_name(path);
	ResourceData* resourceData = _resourceDataTable->get_resource_data(uuidByName);
	io::IFile* oldFile = resourceData->file;
	ecore::Level* oldLevel = static_cast<ecore::Level*>(resourceData->object);
	
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::IFile* newFile = new io::LevelFile(path);
	newFile->deserialize(data, size);
	_fileSystem->unmap_after_reading(data);
	
	ecore::Level* newLevel = new ecore::Level();
	newLevel->deserialize(newFile);

	resourceData->file = newFile;
	resourceData->object = newLevel;
	// TODO think about refreshing metadata name

	delete oldFile;
	delete oldLevel;
	
	return newLevel;
}

void resource::ResourceManager::save_resources()
{
	LOG_INFO("Before saving table")
	_resourceDataTable->save_table();
	LOG_INFO("Before sabing resources in the ResourceManager")
	_resourceDataTable->save_resources();
	LOG_INFO("After sabing resources in the ResourceManager")
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
