#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/shader.h"
#include "engine_core/level/level.h"

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

	class OpaquePBRMaterialCreatedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(OpaquePBRMaterialCreatedEvent)
			OpaquePBRMaterialCreatedEvent(ecore::OpaquePBRMaterial* material) : _materialHandle(material) { }

			ecore::OpaquePBRMaterialHandle get_material_handle() { return _materialHandle; }

		private:
			ecore::OpaquePBRMaterialHandle _materialHandle;
	};

	class ModelCreatedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelCreatedEvent)
			ModelCreatedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};

	class ModelRecreatedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelRecreatedEvent)
			ModelRecreatedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};
	
	class ModelLoadedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelLoadedEvent)
			ModelLoadedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};

	class TextureCreatedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureCreatedEvent)
			TextureCreatedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};

	class TextureRecreatedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureRecreatedEvent)
			TextureRecreatedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};
	
	class TextureLoadedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureLoadedEvent)
			TextureLoadedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};

	class LevelCreatedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Level>
	{
		public:
			EVENT_TYPE_DECL(LevelCreatedEvent)
			LevelCreatedEvent(ecore::Level* level) : ResourceEvent(level) { }
	};

	class LevelLoadedEvent : public events::IEvent, public impl::ResourceEvent<ecore::Level>
	{
		public:
		EVENT_TYPE_DECL(LevelLoadedEvent)
		LevelLoadedEvent(ecore::Level* level) : ResourceEvent(level) { }
	};
}
