#include "module_objects.h"
#include "engine_core/default_object_creators.h"

using namespace ad_astris::engine::impl;

void ModuleObjects::init_engine_object_creators()
{
	ecore::StaticModelCreator::init();
	ecore::SkeletalModelCreator::init();
	ecore::StaticPointLightCreator::init();
	ecore::StaticDirectionalLightCreator::init();
	ecore::StaticSpotLightCreator::init();
	ecore::CameraCreator::init();
}
