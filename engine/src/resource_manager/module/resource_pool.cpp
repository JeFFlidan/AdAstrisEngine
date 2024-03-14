#include "resource_pool.h"
#include "engine_core/model/model.h"
#include "engine_core/texture/texture.h"
#include "engine_core/level/level.h"

using namespace ad_astris;
using namespace resource::impl;

ResourcePool::ResourcePool()
{
	create_pool_for_new_resource<ecore::Model>(512);
	create_pool_for_new_resource<ecore::Texture>(256);
	create_pool_for_new_resource<ecore::Level>(8);
	create_pool_for_new_resource<ecore::ObjectName>(1024);
}
