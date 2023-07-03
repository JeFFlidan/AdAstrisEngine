#include "resource_manager.h"
#include "engine_core/material/general_material_template.h"
#include "profiler/logger.h"
#include "utils.h"

#include <lz4.h>
#include <inicpp.h>

using namespace ad_astris;
using namespace resource;

ResourceManager::ResourceManager(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_resourceConverter = ResourceConverter(_fileSystem);
	_resourceDataTable = new ResourceDataTable(_fileSystem);
	_resourceDataTable->load_table();
}

ResourceManager::~ResourceManager()
{
	delete _resourceDataTable;
}

ResourceAccessor<ecore::Level> ResourceManager::create_level(io::URI& path)
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

ResourceAccessor<ecore::Level> ResourceManager::load_level(io::URI& path)
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

template<>
ResourceAccessor<ecore::GeneralMaterialTemplate> ResourceManager::create_new_resource(
	FirstCreationContext<ecore::GeneralMaterialTemplate> creationContext)
{
	if (_resourceDataTable->check_name_in_table(creationContext.materialTemplateName))
	{
		UUID uuid = _resourceDataTable->get_uuid_by_name(creationContext.materialTemplateName);
		return get_resource<ecore::GeneralMaterialTemplate>(uuid);
	}

	ecore::ShaderUUIDContext uuidContext;
	get_shader_uuid(creationContext.vertexShaderPath, uuidContext);
	get_shader_uuid(creationContext.fragmentShaderPath, uuidContext);
	get_shader_uuid(creationContext.tessControlShader, uuidContext);
	get_shader_uuid(creationContext.tessEvaluationShader, uuidContext);
	get_shader_uuid(creationContext.geometryShader, uuidContext);
	get_shader_uuid(creationContext.computeShader, uuidContext);
	get_shader_uuid(creationContext.meshShader, uuidContext);
	get_shader_uuid(creationContext.taskShader, uuidContext);
	get_shader_uuid(creationContext.rayGenerationShader, uuidContext);
	get_shader_uuid(creationContext.rayIntersectionShader, uuidContext);
	get_shader_uuid(creationContext.rayAnyHitShader, uuidContext);
	get_shader_uuid(creationContext.rayClosestHit, uuidContext);
	get_shader_uuid(creationContext.rayMiss, uuidContext);
	get_shader_uuid(creationContext.rayCallable, uuidContext);
	
	ResourceData resData{};
	resData.object = new ecore::GeneralMaterialTemplate(uuidContext);
	std::string relativePath = "assets/" + creationContext.materialTemplateName + ".aares";
	io::URI absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem, relativePath.c_str());
	io::IFile* newFile = new io::ResourceFile(absolutePath);
	resData.file = newFile;
	resData.metadata.path = absolutePath;
	resData.metadata.type = ResourceType::MATERIAL_TEMPLATE;
	// TODO Do I need name in metadata?

	_resourceDataTable->add_resource(&resData);

	return resData.object;
}

void ResourceManager::save_resources()
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
void ResourceManager::write_to_disk(io::IFile* file, io::URI& originalPath)
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

io::IFile* ResourceManager::read_from_disk(io::URI& path)
{
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::IFile* file = new io::ResourceFile(path);
	file->deserialize(data, size);
	_fileSystem->unmap_after_reading(data);
	return file;
}

UUID ResourceManager::get_shader_uuid(io::URI& shaderPath, ecore::ShaderUUIDContext& shaderContext)
{
	if (!std::string(shaderPath.c_str()).empty())
	{
		return 0;
	}
	
	io::URI absolutePath = shaderPath;
	if (io::Utils::is_relative(shaderPath.c_str()))
	{
		absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem, shaderPath);
	}

	if (!io::Utils::exists(_fileSystem, absolutePath))
	{
		LOG_ERROR("ResourceManager::create_new_resource<GeneralMaterialTemplate>: Path {} is invalid", absolutePath)
		return 0;
	}

	if (_resourceDataTable->check_name_in_table(shaderPath.c_str()))
	{
		return _resourceDataTable->get_uuid_by_name(shaderPath.c_str());
	}

	ResourceData resData;
	resData.metadata.type = ResourceType::SHADER;
	resData.metadata.path = absolutePath;
	resData.metadata.objectName = new ecore::ObjectName(shaderPath.c_str());

	_resourceDataTable->add_resource(&resData);

	return _resourceDataTable->get_uuid_by_name(shaderPath.c_str());
}
