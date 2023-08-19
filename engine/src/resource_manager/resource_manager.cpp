#include "resource_manager.h"
#include "engine_core/material/general_material_template.h"
#include "engine_core/material/shader.h"
#include "profiler/logger.h"
#include "utils.h"

#include <lz4.h>

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
	load_builtin_resources();
	LOG_INFO("ResourceManager::ResourceManager(): Loaded builtin resources")
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
	io::IFile* file = resourceData->file;
	ecore::Level* level = static_cast<ecore::Level*>(resourceData->object);

	if (file)
		return level;
	
	size_t size = 0;
	uint8_t* data = static_cast<uint8_t*>(_fileSystem->map_to_read(path, size, "rb"));
	file = _resourcePool.allocate<io::LevelFile>(path);
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
	for (auto& uuid : _builtinResourcesContext.materialTemplateNames)
	{
		ecore::GeneralMaterialTemplate* materialTemplate = get_resource<ecore::GeneralMaterialTemplate>(uuid).get_resource();
		for (auto& pair : materialTemplate->get_shader_passes())
		{
			ecore::material::ShaderPass& shaderPass = pair.second;
			ecore::material::ShaderUUIDContext& uuidContext = shaderPass.get_shader_uuid_context();
			ecore::material::ShaderHandleContext& handleContext = shaderPass.get_shader_handle_context();
			for (auto& shaderUUID : uuidContext.shaderUUIDs)
			{
				load_shader(shaderUUID, handleContext);
			}
		}
		LOG_INFO("ResourceManager::load_builtin_resources(): Loaded material template: {}", materialTemplate->get_name()->get_full_name())
	}

	_builtinResourcesContext.clear();
}

template <typename T>
ResourceAccessor<T> ResourceManager::create_new_resource(FirstCreationContext<T>& creationContext)
{
	
}

template<>
ResourceAccessor<ecore::GeneralMaterialTemplate> ResourceManager::create_new_resource(
	FirstCreationContext<ecore::GeneralMaterialTemplate>& creationContext)
{
	if (_resourceDataTable.check_name_in_table(creationContext.materialTemplateName))
	{
		UUID uuid = _resourceDataTable.get_uuid_by_name(creationContext.materialTemplateName);
		return get_resource<ecore::GeneralMaterialTemplate>(uuid);
	}

	ecore::material::GeneralMaterialTemplateInfo templateInfo;
	templateInfo.uuid = UUID();
	for (auto& shaderPassInfo : creationContext.shaderPassCreateInfos)
	{
		ecore::material::ShaderUUIDContext uuidContext;
		add_shader_to_uuid_context(shaderPassInfo.vertexShaderPath, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.fragmentShaderPath, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.tessControlShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.tessEvaluationShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.geometryShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.computeShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.meshShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.taskShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayGenerationShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayIntersectionShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayAnyHitShader, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayClosestHit, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayMiss, uuidContext);
		add_shader_to_uuid_context(shaderPassInfo.rayCallable, uuidContext);
		ecore::material::ShaderPass shaderPass(uuidContext, shaderPassInfo.passType);
		ecore::material::ShaderHandleContext& handleContext = shaderPass.get_shader_handle_context();
		for (auto& uuid : shaderPass.get_shader_uuid_context().shaderUUIDs)
		{
			load_shader(uuid, handleContext);
		}
		templateInfo.shaderPassByItsType[shaderPass.get_type()] = shaderPass;
	}
	
	std::string relativePath = "content/builtin/" + creationContext.materialTemplateName + ".aares";
	io::URI absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), relativePath.c_str());
	
	ResourceData resData{};
	resData.metadata.path = absolutePath;
	resData.metadata.type = ResourceType::MATERIAL_TEMPLATE;
	resData.metadata.objectName = _resourcePool.allocate<ecore::ObjectName>(creationContext.materialTemplateName.c_str());
	resData.object = _resourcePool.allocate<ecore::GeneralMaterialTemplate>(templateInfo, resData.metadata.objectName);
	resData.file = _resourcePool.allocate<io::ResourceFile>(absolutePath);

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

void ResourceManager::add_shader_to_uuid_context(io::URI& shaderPath, ecore::material::ShaderUUIDContext& shaderContext)
{
	if (std::string(shaderPath.c_str()).empty())
	{
		return;
	}

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

	if (_resourceDataTable.check_name_in_table(shaderPath.c_str()))
	{
		shaderContext.shaderUUIDs.push_back(_resourceDataTable.get_uuid_by_name(shaderPath.c_str()));
		return;
	}

	ResourceData resData;
	resData.metadata.type = ResourceType::SHADER;
	resData.metadata.path = absolutePath;
	resData.metadata.objectName = _resourcePool.allocate<ecore::ObjectName>(shaderPath.c_str());
	resData.metadata.builtin = true;
	shaderContext.shaderUUIDs.push_back(UUID());
	_resourceDataTable.add_empty_resource(&resData, shaderContext.shaderUUIDs.back());
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
