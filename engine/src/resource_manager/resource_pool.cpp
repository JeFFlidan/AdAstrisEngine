#include "resource_pool.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/materials.h"
#include "engine_core/level/level.h"
#include "engine_core/material/shader.h"
#include "file_system/file.h"

using namespace ad_astris;
using namespace resource;

ResourcePool::ResourcePool()
{
	create_pool_for_new_resource<ecore::StaticModel>();
	create_pool_for_new_resource<ecore::Texture2D>();
	create_pool_for_new_resource<ecore::Level>();
	create_pool_for_new_resource<ecore::Shader>();
	create_pool_for_new_resource<ResourceFile>();
	create_pool_for_new_resource<LevelFile>();
	create_pool_for_new_resource<ecore::ObjectName>();
	create_pool_for_new_resource<ecore::OpaquePBRMaterial>();
	create_pool_for_new_resource<ecore::TransparentMaterial>();
}

void ResourcePool::cleanup()
{
	for (auto& alloc : _poolAllocatorByTypeName)
		alloc.second->cleanup();
}
