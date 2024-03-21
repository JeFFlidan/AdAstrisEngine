#pragma once

#include "engine_core/contexts.h"
#include "engine_core/world.h"

namespace ad_astris::engine::impl
{
	class EngineObjectsCreator
	{
		public:
			EngineObjectsCreator();

			void create_new_objects();
		
		private:
			std::vector<ecore::EditorObjectCreationContext> _staticPointLightsToCreate;
			std::vector<ecore::EditorObjectCreationContext> _staticModelsToCreate;

			void subscribe_to_events();
	};
}