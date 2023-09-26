#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/shader.h"

namespace ad_astris::resource
{
	class StaticModelFirstCreationEvent : public events::IEvent, public impl::StaticModelEvent
	{
		public:
			EVENT_TYPE_DECL(StaticModelFirstCreationEvent)
			StaticModelFirstCreationEvent(ecore::StaticModel* model) : StaticModelEvent(model) { }
	};
	
	class StaticModelLoadedInEngineEvent : public events::IEvent, public impl::StaticModelEvent
	{
		public:
			EVENT_TYPE_DECL(StaticModelLoadedInEngineEvent)
			StaticModelLoadedInEngineEvent(ecore::StaticModel* model) : StaticModelEvent(model) { }
	};

	class Texture2DCreatedEvent : public events::IEvent, public impl::Texture2DEvent
	{
		public:
			EVENT_TYPE_DECL(Texture2DCreatedEvent)
			Texture2DCreatedEvent(ecore::Texture2D* texture) : Texture2DEvent(texture) { }
	};

	class Texture2DLoadedEvent : public events::IEvent, public impl::Texture2DEvent
	{
		public:
			EVENT_TYPE_DECL(Texture2DLoadedEvent)
			Texture2DLoadedEvent(ecore::Texture2D* texture) : Texture2DEvent(texture) { }
	};
}
