#pragma once

#include "engine_core/contexts.h"
#include "engine_core/world.h"

namespace ad_astris::ecore
{
#define DEFINE_ENGINE_OBJECT_CREATOR() \
	public:								\
		static void init();				\
		static ecs::Entity create(ecore::EditorObjectCreationContext& objectCreationContext);
	
	class StaticModelCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};

	class SkeletalModelCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};

	class StaticPointLightCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};

	class StaticDirectionalLightCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};

	class StaticSpotLightCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};
	
	class CameraCreator
	{
		DEFINE_ENGINE_OBJECT_CREATOR();
	};
}