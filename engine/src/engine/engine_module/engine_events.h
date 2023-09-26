#pragma once

// Local events only for the engine module

#include "events/event.h"
#include "ecs/entity_types.h"

namespace ad_astris::engine::impl
{
	class UpdateActiveCameraEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(UpdateActiveCameraEvent)
			UpdateActiveCameraEvent(ecs::Entity entity) : _entity(entity) { }

			ecs::Entity get_entity() { return _entity; }

		private:
			ecs::Entity _entity;
	};
}
