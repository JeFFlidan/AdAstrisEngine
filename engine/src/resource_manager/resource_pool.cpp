﻿#include "resource_pool.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/general_material_template.h"
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
	create_pool_for_new_resource<ecore::GeneralMaterialTemplate>();
	create_pool_for_new_resource<ecore::Shader>();
	create_pool_for_new_resource<io::ResourceFile>();
	create_pool_for_new_resource<io::LevelFile>();
	create_pool_for_new_resource<ecore::ObjectName>();
}

void ResourcePool::cleanup()
{
	for (auto& alloc : _poolAllocatorByTypeName)
		alloc.second->cleanup();
}