#include "resource_manager.h"

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

void impl::ResourceManager::init(const experimental::ResourceManagerInitContext& initContext)
{
	assert(initContext.globalObjectContext);
	GlobalObjects::set_global_object_context(initContext.globalObjectContext);
	_resourcePool = std::make_unique<ResourcePool>();
	_resourceTable = std::make_unique<ResourceTable>(_resourcePool.get());
}

void impl::ResourceManager::cleanup()
{
	
}

UUID impl::ResourceManager::convert_to_engine_format(const io::URI& originalResourcePath, const io::URI& engineResourcePath)
{
	return UUID();
}

void impl::ResourceManager::save_resources() const
{
	
}

void impl::ResourceManager::destroy_resource(UUID uuid)
{
	
}

ResourceAccessor<ecore::Model> impl::ResourceManager::get_model(UUID uuid) const
{
	return ResourceAccessor<ecore::Model>();
}

ResourceAccessor<ecore::Model> impl::ResourceManager::get_model(const std::string& modelName) const
{
	return ResourceAccessor<ecore::Model>();
}

ResourceAccessor<ecore::Texture> impl::ResourceManager::get_texture(UUID uuid) const
{
	return ResourceAccessor<ecore::Texture>();
}

ResourceAccessor<ecore::Texture> impl::ResourceManager::get_texture(const std::string& textureName) const
{
	return ResourceAccessor<ecore::Texture>();
}

ResourceAccessor<ecore::Level> impl::ResourceManager::get_level(UUID uuid) const
{
	return ResourceAccessor<ecore::Level>();
}

ResourceAccessor<ecore::Level> impl::ResourceManager::get_level(const std::string& levelName) const
{
	return ResourceAccessor<ecore::Level>();
}

ResourceType impl::ResourceManager::get_resource_type(UUID uuid) const
{
	return ResourceType::UNDEFINED;
}

std::string impl::ResourceManager::get_resource_name(UUID uuid) const
{
	return "undefined";
}

UUID impl::ResourceManager::get_resource_uuid(const std::string& resourceName) const
{
	return UUID();
}

bool impl::ResourceManager::is_resource_loaded(UUID uuid) const
{
	return false;
}

bool impl::ResourceManager::is_supported_model_format(const std::string& extension) const
{
	auto it = SUPPORTED_MODEL_FORMATS.find(extension);
	if (it == SUPPORTED_MODEL_FORMATS.end())
		return false;
	return true;
}

bool impl::ResourceManager::is_supported_texture_format(const std::string& extension) const
{
	auto it = SUPPORTED_IMAGE_FORMATS.find(extension);
	if (it == SUPPORTED_IMAGE_FORMATS.end())
		return false;
	return true;
}
