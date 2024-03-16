#include "resource_pool.h"
#include "engine_core/model/model.h"
#include "engine_core/texture/texture.h"
#include "engine_core/level/level.h"
#include "engine_core/material/material.h"
#include "engine_core/script/script.h"
#include "engine_core/video/video.h"
#include "engine_core/font/font.h"
#include "engine_core/audio/sound.h"

using namespace ad_astris;
using namespace resource::impl;

ResourcePool::ResourcePool()
{
	create_pool_for_new_resource<ecore::Model>(512);
	create_pool_for_new_resource<ecore::Texture>(256);
	create_pool_for_new_resource<ecore::Level>(8);
	create_pool_for_new_resource<ecore::Material>(256);
	create_pool_for_new_resource<ecore::Script>(128);
	create_pool_for_new_resource<ecore::Video>(32);
	create_pool_for_new_resource<ecore::Font>(16);
	create_pool_for_new_resource<ecore::Sound>(256);
	create_pool_for_new_resource<ecore::ObjectName>(1024);
}
