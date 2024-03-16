#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/shader.h"
#include "engine_core/level/level.h"
#include "engine_core/model/model.h"
#include "engine_core/material/material.h"
#include "engine_core/video/video.h"
#include "engine_core/script/script.h"
#include "engine_core/texture/texture.h"
#include "engine_core/font/font.h"
#include "engine_core/audio/sound.h"

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

	DECL_RESOURCE_EVENT(ModelCreatedEvent, ecore::Model)
	DECL_RESOURCE_EVENT(ModelRecreatedEvent, ecore::Model)
	DECL_RESOURCE_EVENT(ModelLoadedEvent, ecore::Model)
	DECL_RESOURCE_EVENT(TextureCreatedEvent, ecore::Texture)
	DECL_RESOURCE_EVENT(TextureRecreatedEvent, ecore::Texture)
	DECL_RESOURCE_EVENT(TextureLoadedEvent, ecore::Texture)
	DECL_RESOURCE_EVENT(LevelCreatedEvent, ecore::Level)
	DECL_RESOURCE_EVENT(LevelLoadedEvent, ecore::Level)
	DECL_RESOURCE_EVENT(MaterialCreatedEvent, ecore::Material)
	DECL_RESOURCE_EVENT(MaterialLoadedEvent, ecore::Material)
	DECL_RESOURCE_EVENT(ScriptCreatedEvent, ecore::Script)
	DECL_RESOURCE_EVENT(ScriptLoadedEvent, ecore::Script)
	DECL_RESOURCE_EVENT(VideoCreatedEvent, ecore::Video)
	DECL_RESOURCE_EVENT(VideoRecreatedEvent, ecore::Video)
	DECL_RESOURCE_EVENT(VideoLoadedEvent, ecore::Video)
	DECL_RESOURCE_EVENT(FontCreatedEvent, ecore::Font)
	DECL_RESOURCE_EVENT(FontRecreatedEvent, ecore::Font)
	DECL_RESOURCE_EVENT(FontLoadedEvent, ecore::Font)
	DECL_RESOURCE_EVENT(SoundCreatedEvent, ecore::Sound)
	DECL_RESOURCE_EVENT(SoundRecreatedEvent, ecore::Sound)
	DECL_RESOURCE_EVENT(SoundLoadedEvent, ecore::Sound)
}
