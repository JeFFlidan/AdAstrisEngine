#include "resource_manager.h"
#include "model_importer.h"
#include "texture_importer.h"
#include "resource_manager/utils.h"
#include "resource_manager/resource_events.h"

using namespace ad_astris;
using namespace resource;

const std::unordered_set<std::string> SUPPORTED_MODEL_FORMATS = {
	"gltf",
	"glb",
	"obj"
};

const std::unordered_set<std::string> SUPPORTED_IMAGE_FORMATS = {
	"tga",
	"png",
	"tiff",
	"jpg"
};

const std::unordered_set<std::string> SUPPORTED_SCRIPT_FORMATS = { "lua" };
const std::unordered_set<std::string> SUPPORTED_VIDEO_FORMATS = { "mp4" };
const std::unordered_set<std::string> SUPPORTED_FONT_FORMATS = { "ttf" };
const std::unordered_set<std::string> SUPPORTED_SOUND_FORMATS = { "wav", "ogg" };

void impl::ResourceManager::init(const experimental::ResourceManagerInitContext& initContext)
{
	assert(initContext.globalObjectContext);
	GlobalObjects::set_global_object_context(initContext.globalObjectContext);
	_resourcePool = std::make_unique<ResourcePool>();
	_resourceTable = std::make_unique<ResourceTable>(_resourcePool.get());
	TextureImporter::init();
}

void impl::ResourceManager::cleanup()
{
	_resourcePool->cleanup();
}

template<typename Resource>
constexpr ResourceType get_resource_type()
{
	if constexpr (std::is_same_v<ResourceType, ecore::Model>)
		return ResourceType::MODEL;
	if constexpr (std::is_same_v<ResourceType, ecore::Texture>)
		return ResourceType::TEXTURE;
	return ResourceType::UNDEFINED;
}

enum class EventType
{
	CREATE,
	RECREATE
};

template<typename Resource, EventType eventType>
void enqueue_event(ecore::Object* resource)
{
	constexpr ResourceType resourceType = get_resource_type<Resource>(); 
	
	switch (eventType)
	{
		case EventType::CREATE:
		{
			ResourceCreatedEvent<Resource> event(static_cast<Resource*>(resource));
			EVENT_MANAGER()->enqueue_event(event);
			break;
		}
		case EventType::RECREATE:
		{
			ResourceRecreatedEvent<Resource> event(static_cast<Resource*>(resource));
			EVENT_MANAGER()->enqueue_event(event);
			break;
		}
	}
}

template<typename Resource, typename ResourceInfo>
UUID add_resource_to_table(
	impl::ResourceTable* resourceTable,
	const ResourceInfo& resourceInfo,
	const std::string& resourceName,
	const io::URI& engineResourcePath)
{
	constexpr ResourceType resourceType = get_resource_type<Resource>();
	
	if (resourceTable->is_resource_desc_valid(resourceName))
	{
		UUID uuid = resourceTable->get_resource_uuid(resourceName);
		impl::ResourceDesc* resourceDesc = resourceTable->get_resource_desc(uuid);
		switch (resourceDesc->type)
		{
			case resourceType:
			{
				Resource* resource = static_cast<Resource*>(resourceDesc->resource);
				resource->set_info(resourceInfo);
				enqueue_event<Resource, EventType::RECREATE>(resource);
				return uuid;
			}
			default:
			{
				LOG_ERROR("ResourceManager::add_resource_to_table(): Ambigious names for resources of different types")
				return {};
			}
		}
	}

	impl::ResourceDesc resourceDesc;
	resourceDesc.type = resourceType;
	resourceDesc.resourceName = resourceTable->get_resource_pool()->allocate<ecore::ObjectName>(resourceName.c_str());
	resourceDesc.path = engineResourcePath + "/" + resourceDesc.resourceName->get_full_name() + ".aares";
	resourceDesc.resource = resourceTable->get_resource_pool()->allocate<Resource>(resourceInfo, resourceDesc.resourceName);
	resourceDesc.resource->make_dirty();
	enqueue_event<Resource, EventType::CREATE>(resourceDesc.resource);
	resourceTable->add_resource(resourceDesc);
	resourceTable->save_resource(resourceDesc.resource->get_uuid());
	return resourceDesc.resource->get_uuid();
}

std::vector<UUID> impl::ResourceManager::convert_to_engine_format(
	const io::URI& originalResourcePath,
	const io::URI& engineResourcePath,
	void* conversionContext)
{
	if (!io::Utils::exists(FILE_SYSTEM(), originalResourcePath))
	{
		LOG_ERROR("ResourceManager::convert_to_engine_format(): No file with path {}", originalResourcePath.c_str())
		return {};
	}

	if (io::Utils::is_relative(originalResourcePath))
	{
		LOG_ERROR("ResourceManager::convert_to_engine_format(): Can't use relative path {}", originalResourcePath.c_str())
		return {};
	}
	
	ResourceType resourceType{ ResourceType::UNDEFINED };
	std::string extension = io::Utils::get_file_extension(originalResourcePath);
	if (is_model_format_supported(extension))
		resourceType = ResourceType::MODEL;
	else if (is_texture_format_supported(extension))
		resourceType = ResourceType::TEXTURE;
	else if (is_font_format_supported(extension))
		resourceType = ResourceType::FONT;
	else if (is_video_format_supported(extension))
		resourceType = ResourceType::VIDEO;
	else if (is_sound_format_supported(extension))
		resourceType = ResourceType::SOUND;

	std::vector<UUID> outputUUIDs;

	switch (resourceType)
	{
		case ResourceType::MODEL:
		{
			if (!conversionContext)
			{
				LOG_ERROR("ResourceManager::convert_to_engine_format(): ModelConversionContext is nullptr")
				return {};
			}
			ModelImportContext importContext(conversionContext);
			ModelImporter::import(originalResourcePath, importContext);

			for (auto& modelInfo : importContext.modelCreateInfos)
			{
				outputUUIDs.push_back(add_resource_to_table<ecore::Model>(
					_resourceTable.get(),
					modelInfo.info,
					modelInfo.name.empty() ? io::Utils::get_file_name(originalResourcePath) : modelInfo.name,
					engineResourcePath));
			}

			for (auto& textureInfo : importContext.textureCreateInfos)
			{
				outputUUIDs.push_back(add_resource_to_table<ecore::Texture>(
					_resourceTable.get(),
					textureInfo.info,
					textureInfo.name,
					engineResourcePath));
			}
			
			for (auto& materialInfo : importContext.materialCreateInfos)
			{
				outputUUIDs.push_back(add_resource_to_table<ecore::Material>(
					_resourceTable.get(),
					materialInfo.info,
					materialInfo.name,
					engineResourcePath));
			}

			break;
		}
		case ResourceType::TEXTURE:
		{
			ecore::TextureInfo textureInfo;
			TextureImporter::import(originalResourcePath.c_str(), textureInfo);
			outputUUIDs.push_back(add_resource_to_table<ecore::Texture>(
				_resourceTable.get(),
				textureInfo,
				io::Utils::get_file_name(originalResourcePath),
				engineResourcePath));
			break;
		}
		case ResourceType::VIDEO:
		{
			// TODO
			break;
		}
		case ResourceType::FONT:
		{
			size_t size = 0;
			uint8_t* data = static_cast<uint8_t*>(FILE_SYSTEM()->map_to_read(originalResourcePath, size));
			
			ecore::FontInfo fontInfo;
			fontInfo.init(data, size);
			outputUUIDs.push_back(add_resource_to_table<ecore::Font>(
				_resourceTable.get(),
				fontInfo,
				io::Utils::get_file_name(originalResourcePath),
				engineResourcePath));
			
			FILE_SYSTEM()->unmap_after_reading(data);
			break;
		}
		case ResourceType::SOUND:
		{
			// TODO
			break;
		}
		default:
		{
			LOG_ERROR("ResourceManager::convert_to_engine_format(): Resource with type {} can't be converted to engine format", Utils::get_str_resource_type(resourceType))
			return {};
		}
	}
	
	return outputUUIDs;
}

void impl::ResourceManager::save_resources() const
{
	_resourceTable->save_config();
	_resourceTable->save_resources();
}

void impl::ResourceManager::save_resource(UUID uuid) const
{
	_resourceTable->save_resource(uuid);
}

void impl::ResourceManager::save_resource(const std::string& name) const
{
	_resourceTable->save_resource(get_resource_uuid(name));
}

void impl::ResourceManager::unload_resource(UUID uuid)
{
	_resourceTable->unload_resource(uuid);
}

void impl::ResourceManager::unload_resource(const std::string& name)
{
	_resourceTable->unload_resource(get_resource_uuid(name));
}

void impl::ResourceManager::destroy_resource(UUID uuid)
{
	_resourceTable->destroy_resource(uuid);
}

void impl::ResourceManager::destroy_resource(const std::string& name)
{
	_resourceTable->destroy_resource(get_resource_uuid(name));
}

ResourceAccessor<ecore::Level> impl::ResourceManager::create_level(const ecore::LevelCreateInfo& createInfo)
{
	ResourceDesc resourceDesc;
	resourceDesc.type = ResourceType::LEVEL;
	resourceDesc.path = createInfo.path + "/" + createInfo.name + ".aalevel";
	resourceDesc.resourceName = _resourcePool->allocate<ecore::ObjectName>(createInfo.name.c_str());
	ecore::LevelInfo levelInfo;
	resourceDesc.resource = _resourcePool->allocate<ecore::Level>(levelInfo, resourceDesc.resourceName);
	_resourceTable->add_resource(resourceDesc);
	_resourceTable->save_resource(resourceDesc.resource->get_uuid());
	return resourceDesc.resource;
}

ResourceAccessor<ecore::MaterialTemplate> impl::ResourceManager::create_material_template(const ecore::MaterialTemplateCreateInfo& createInfo)
{
	ResourceDesc resourceDesc;
	resourceDesc.type = ResourceType::MATERIAL_TEMPLATE;
	resourceDesc.path = createInfo.resourceFolderPath + "/" + createInfo.name + ".aares";
	resourceDesc.resourceName = _resourcePool->allocate<ecore::ObjectName>(createInfo.name.c_str());
	resourceDesc.resource = _resourcePool->allocate<ecore::MaterialTemplate>(createInfo, resourceDesc.resourceName);
	_resourceTable->add_resource(resourceDesc);
	_resourceTable->save_resource(resourceDesc.resource->get_uuid());
	return resourceDesc.resource;
}

ResourceAccessor<ecore::Material> impl::ResourceManager::create_material(const ecore::MaterialCreateInfo& createInfo)
{
	return nullptr;
}

ResourceAccessor<ecore::Script> impl::ResourceManager::create_script(const ecore::ScriptCreateInfo& createInfo)
{
	return nullptr;
}

ResourceAccessor<ecore::Model> impl::ResourceManager::get_model(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Model>(uuid, ResourceType::MODEL);
}

ResourceAccessor<ecore::Model> impl::ResourceManager::get_model(const std::string& modelName) const
{
	return _resourceTable->load_resource<ecore::Model>(get_resource_uuid(modelName), ResourceType::MODEL);
}

ResourceAccessor<ecore::Texture> impl::ResourceManager::get_texture(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Texture>(uuid, ResourceType::TEXTURE);
}

ResourceAccessor<ecore::Texture> impl::ResourceManager::get_texture(const std::string& textureName) const
{
	return _resourceTable->load_resource<ecore::Texture>(get_resource_uuid(textureName), ResourceType::TEXTURE);
}

ResourceAccessor<ecore::Level> impl::ResourceManager::get_level(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Level>(uuid, ResourceType::LEVEL);
}

ResourceAccessor<ecore::Level> impl::ResourceManager::get_level(const std::string& levelName) const
{
	return _resourceTable->load_resource<ecore::Level>(get_resource_uuid(levelName), ResourceType::LEVEL);
}

ResourceAccessor<ecore::Material> impl::ResourceManager::get_material(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Material>(uuid, ResourceType::MATERIAL);
}

ResourceAccessor<ecore::Material> impl::ResourceManager::get_material(const std::string& materialName) const
{
	return _resourceTable->load_resource<ecore::Material>(get_resource_uuid(materialName), ResourceType::MATERIAL);
}

ResourceAccessor<ecore::MaterialTemplate> impl::ResourceManager::get_material_template(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::MaterialTemplate>(uuid, ResourceType::MATERIAL_TEMPLATE);
}

ResourceAccessor<ecore::MaterialTemplate> impl::ResourceManager::get_material_template(const std::string& materialTemplateName) const
{
	return _resourceTable->load_resource<ecore::MaterialTemplate>(get_resource_uuid(materialTemplateName), ResourceType::MATERIAL_TEMPLATE);
}

ResourceAccessor<ecore::Script> impl::ResourceManager::get_script(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Script>(uuid, ResourceType::SCRIPT);
}

ResourceAccessor<ecore::Script> impl::ResourceManager::get_script(const std::string& scriptName) const
{
	return _resourceTable->load_resource<ecore::Script>(get_resource_uuid(scriptName), ResourceType::SCRIPT);
}

ResourceAccessor<ecore::Video> impl::ResourceManager::get_video(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Video>(uuid, ResourceType::VIDEO);
}

ResourceAccessor<ecore::Video> impl::ResourceManager::get_video(const std::string& videoName) const
{
	return _resourceTable->load_resource<ecore::Video>(get_resource_uuid(videoName), ResourceType::VIDEO); 
}

ResourceAccessor<ecore::Font> impl::ResourceManager::get_font(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Font>(uuid, ResourceType::FONT);
}

ResourceAccessor<ecore::Font> impl::ResourceManager::get_font(const std::string& fontName) const
{
	return _resourceTable->load_resource<ecore::Font>(get_resource_uuid(fontName), ResourceType::FONT); 
}

ResourceAccessor<ecore::Sound> impl::ResourceManager::get_sound(UUID uuid) const
{
	return _resourceTable->load_resource<ecore::Sound>(uuid, ResourceType::SOUND);
}

ResourceAccessor<ecore::Sound> impl::ResourceManager::get_sound(const std::string& soundName) const
{
	return _resourceTable->load_resource<ecore::Sound>(get_resource_uuid(soundName), ResourceType::SOUND);
}

ResourceType impl::ResourceManager::get_resource_type(UUID uuid) const
{
	return _resourceTable->get_resource_type(uuid);
}

std::string impl::ResourceManager::get_resource_name(UUID uuid) const
{
	return _resourceTable->get_resource_name(uuid)->get_full_name();
}

UUID impl::ResourceManager::get_resource_uuid(const std::string& resourceName) const
{
	return _resourceTable->get_resource_uuid(resourceName);
}

bool impl::ResourceManager::is_resource_loaded(UUID uuid) const
{
	return _resourceTable->is_resource_loaded(uuid);
}

bool impl::ResourceManager::is_model_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_MODEL_FORMATS.find(extension);
	if (it == SUPPORTED_MODEL_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_texture_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_IMAGE_FORMATS.find(extension);
	if (it == SUPPORTED_IMAGE_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_script_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_SCRIPT_FORMATS.find(extension);
	if (it == SUPPORTED_SCRIPT_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_video_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_VIDEO_FORMATS.find(extension);
	if (it == SUPPORTED_VIDEO_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_font_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_FONT_FORMATS.find(extension);
	if (it == SUPPORTED_FONT_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_sound_format_supported(const std::string& extension) const
{
	auto it = SUPPORTED_SOUND_FORMATS.find(extension);
	if (it == SUPPORTED_SOUND_FORMATS.end())
		return false;
	return true;
}
