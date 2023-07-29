#include "resource_manager.h"
#include "engine_core/material/general_material_template.h"
#include "engine_core/material/shader.h"
#include "profiler/logger.h"
#include "utils.h"

#include <lz4.h>
#include <inicpp.h>

using namespace ad_astris;
using namespace resource;

ResourceManager::ResourceManager(io::FileSystem* fileSystem, events::EventManager* eventManager)
	: _fileSystem(fileSystem), _eventManager(eventManager)
{
	LOG_INFO("Before initing resource converter")
	_resourceConverter = ResourceConverter(_fileSystem);
	LOG_INFO("Before initing resource pool")
	_resourcePool = ResourcePool();
	LOG_INFO("Before initing table")
	_resourceDataTable = ResourceDataTable(_fileSystem, &_resourcePool);
	LOG_INFO("Before loading")
	_resourceDataTable.load_table(_builtinResourcesContext);
	LOG_INFO("Before loading builtin resources")
	load_builtin_resources();
	LOG_INFO("After loading builtin resources")
}

ResourceManager::~ResourceManager()
{
	_resourcePool.cleanup();
}

ResourceAccessor<ecore::Level> ResourceManager::create_level(io::URI& path)
{
	std::string strLevelName = io::Utils::get_file_name(path);
	ecore::ObjectName* levelName = _resourcePool.allocate<ecore::ObjectName>(strLevelName.c_str());
	io::IFile* levelFile = _resourcePool.allocate<io::LevelFile>(path);
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
	io::IFile* oldFile = resourceData->file;
	ecore::Level* oldLevel = static_cast<ecore::Level*>(resourceData->object);
	
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::IFile* newFile = _resourcePool.allocate<io::LevelFile>(path);
	newFile->deserialize(data, size);
	_fileSystem->unmap_after_reading(data);
	
	ecore::Level* newLevel = _resourcePool.allocate<ecore::Level>();
	newLevel->deserialize(newFile, resourceData->metadata.objectName);

	resourceData->file = newFile;
	resourceData->object = newLevel;

	// TODO Think if I need this deletion
	delete oldFile;
	delete oldLevel;
	
	return newLevel;
}

void ResourceManager::load_builtin_resources()
{
	for (auto& uuid : _builtinResourcesContext.materialTemplateNames)
	{
		ecore::GeneralMaterialTemplate* materialTemplate = get_resource<ecore::GeneralMaterialTemplate>(uuid).get_resource();
		LOG_INFO("Template name: {}", materialTemplate->get_name()->get_full_name())
		ecore::material::ShaderUUIDContext& uuidContext = materialTemplate->get_shader_uuid_context();
		ecore::material::ShaderHandleContext& handleContext = materialTemplate->get_shader_handle_context();
		for (auto& shaderUUID : uuidContext.shaderUUIDs)
		{
			load_shader(shaderUUID, handleContext);
		}
		LOG_INFO("Load material template: {}", materialTemplate->get_name()->get_full_name())
	}

	_builtinResourcesContext.clear();
}

template <typename T>
ResourceAccessor<T> ResourceManager::create_new_resource(FirstCreationContext<T> creationContext)
{
	
}

template<>
ResourceAccessor<ecore::GeneralMaterialTemplate> ResourceManager::create_new_resource(
	FirstCreationContext<ecore::GeneralMaterialTemplate> creationContext)
{
	if (_resourceDataTable.check_name_in_table(creationContext.materialTemplateName))
	{
		UUID uuid = _resourceDataTable.get_uuid_by_name(creationContext.materialTemplateName);
		return get_resource<ecore::GeneralMaterialTemplate>(uuid);
	}

	ecore::material::ShaderUUIDContext uuidContext;
	add_shader_uuid_to_context(creationContext.vertexShaderPath, uuidContext);
	add_shader_uuid_to_context(creationContext.fragmentShaderPath, uuidContext);
	add_shader_uuid_to_context(creationContext.tessControlShader, uuidContext);
	add_shader_uuid_to_context(creationContext.tessEvaluationShader, uuidContext);
	add_shader_uuid_to_context(creationContext.geometryShader, uuidContext);
	add_shader_uuid_to_context(creationContext.computeShader, uuidContext);
	add_shader_uuid_to_context(creationContext.meshShader, uuidContext);
	add_shader_uuid_to_context(creationContext.taskShader, uuidContext);
	add_shader_uuid_to_context(creationContext.rayGenerationShader, uuidContext);
	add_shader_uuid_to_context(creationContext.rayIntersectionShader, uuidContext);
	add_shader_uuid_to_context(creationContext.rayAnyHitShader, uuidContext);
	add_shader_uuid_to_context(creationContext.rayClosestHit, uuidContext);
	add_shader_uuid_to_context(creationContext.rayMiss, uuidContext);
	add_shader_uuid_to_context(creationContext.rayCallable, uuidContext);
	
	std::string relativePath = "assets/" + creationContext.materialTemplateName + ".aares";
	io::URI absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem, relativePath.c_str());
	
	ResourceData resData{};
	resData.metadata.path = absolutePath;
	resData.metadata.type = ResourceType::MATERIAL_TEMPLATE;
	resData.metadata.objectName = _resourcePool.allocate<ecore::ObjectName>(creationContext.materialTemplateName.c_str());
	resData.object = _resourcePool.allocate<ecore::GeneralMaterialTemplate>(uuidContext, resData.metadata.objectName);
	resData.file = _resourcePool.allocate<io::ResourceFile>(absolutePath);

	_resourceDataTable.add_resource(&resData);

	return resData.object;
}

void ResourceManager::save_resources()
{
	LOG_INFO("Before saving table")
	_resourceDataTable.save_table();
	LOG_INFO("Before sabing resources in the ResourceManager")
	_resourceDataTable.save_resources();
	LOG_INFO("After saving resources in the ResourceManager")
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

io::IFile* ResourceManager::read_from_disk(io::URI& path, bool isShader)
{
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	io::IFile* file = _resourcePool.allocate<io::ResourceFile>(path);
	
	if (isShader)
		file->set_binary_blob(data, size);
	else
		file->deserialize(data, size);
	
	_fileSystem->unmap_after_reading(data);
	return file;
}

void ResourceManager::add_shader_uuid_to_context(io::URI& shaderPath, ecore::material::ShaderUUIDContext& shaderContext)
{
	if (std::string(shaderPath.c_str()).empty())
	{
		return;
	}
	LOG_INFO("Shader path: {}", shaderPath.c_str())
	io::URI absolutePath = shaderPath;
	if (io::Utils::is_relative(shaderPath.c_str()))
	{
		absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem, shaderPath);
	}

	if (!io::Utils::exists(_fileSystem, absolutePath))
	{
		LOG_ERROR("ResourceManager::create_new_resource<GeneralMaterialTemplate>: Path {} is invalid", absolutePath.c_str());
		return;
	}

	LOG_INFO("Absolute path {}", absolutePath.c_str())

	if (_resourceDataTable.check_name_in_table(shaderPath.c_str()))
	{
		UUID uuid = _resourceDataTable.get_uuid_by_name(shaderPath.c_str());
		shaderContext.shaderUUIDs.push_back(uuid);
		return;
	}

	ResourceData resData;
	resData.metadata.type = ResourceType::SHADER;
	resData.metadata.path = absolutePath;
	resData.metadata.objectName = _resourcePool.allocate<ecore::ObjectName>(shaderPath.c_str());
	resData.object = _resourcePool.allocate<ecore::Shader>(resData.metadata.objectName);

	_resourceDataTable.add_resource(&resData);
	shaderContext.shaderUUIDs.push_back(resData.object->get_uuid());
}

void ResourceManager::load_shader(UUID& shaderUUID, ecore::material::ShaderHandleContext& shaderContext)
{
	ResourceAccessor<ecore::Shader> shaderHandle = get_resource<ecore::Shader>(shaderUUID);
	ecore::Shader* shaderObject = shaderHandle.get_resource();
	if (shaderObject->get_shader_type() == rhi::ShaderType::UNDEFINED)
	{
		LOG_ERROR("GeneralMaterialTemplate::load_required_shaders(): Shader {} has undefined type.", shaderObject->get_path().c_str())
		return;
	}

	switch (shaderObject->get_shader_type())
	{
		case rhi::ShaderType::VERTEX:
			shaderContext.vertexShader = shaderHandle;
			break;
		case rhi::ShaderType::FRAGMENT:
			shaderContext.fragmentShader = shaderHandle;
			break;
		case rhi::ShaderType::TESSELLATION_CONTROL:
			shaderContext.tessControlShader = shaderHandle;
			break;
		case rhi::ShaderType::TESSELLATION_EVALUATION:
			shaderContext.tessEvaluationShader = shaderHandle;
			break;
		case rhi::ShaderType::GEOMETRY:
			shaderContext.geometryShader = shaderHandle;
			break;
		case rhi::ShaderType::COMPUTE:
			shaderContext.computeShader = shaderHandle;
			break;
		case rhi::ShaderType::MESH:
			shaderContext.meshShader = shaderHandle;
			break;
		case rhi::ShaderType::TASK:
			shaderContext.taskShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_GENERATION:
			shaderContext.rayGenerationShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_INTERSECTION:
			shaderContext.rayIntersectionShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_ANY_HIT:
			shaderContext.rayAnyHitShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_CLOSEST_HIT:
			shaderContext.rayClosestHitShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_MISS:
			shaderContext.rayMissShader = shaderHandle;
			break;
		case rhi::ShaderType::RAY_CALLABLE:
			shaderContext.rayClosestHitShader = shaderHandle;
			break;
	}
}

template <typename T>
void ResourceManager::send_resource_event(T* resourceObject)
{
	
}

template<>
void ResourceManager::send_resource_event(ecore::StaticModel* resourceObject)
{
	LOG_INFO("Sending static model event")
	StaticModelLoadedEvent event(resourceObject);
	_eventManager->trigger_event(event);
}

template<>
void ResourceManager::send_resource_event(ecore::Texture2D* resourceObject)
{
	LOG_INFO("Sending texture2D event")
	Texture2DLoadedEvent event(resourceObject);
	_eventManager->trigger_event(event);
}

void BuiltinResourcesContext::clear()
{
	materialTemplateNames.clear();
}
