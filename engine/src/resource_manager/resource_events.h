#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/shader.h"
#include "engine_core/level/level.h"
#include "engine_core/model/model.h"
#include "engine_core/material/material.h"
#include "engine_core/material/material_template.h"
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

	template<>
	class ResourceCreatedEvent<ecore::Model> : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelCreatedEvent)
			ResourceCreatedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};

	template<>
	class ResourceRecreatedEvent<ecore::Model> : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelRecreatedEvent)
			ResourceRecreatedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::Model> : public events::IEvent, public impl::ResourceEvent<ecore::Model>
	{
		public:
			EVENT_TYPE_DECL(ModelLoadedEvent)
			ResourceLoadedEvent(ecore::Model* model) : ResourceEvent(model) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Texture> : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureCreatedEvent)
			ResourceCreatedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};

	template<>
	class ResourceRecreatedEvent<ecore::Texture> : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureRecreatedEvent)
			ResourceRecreatedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::Texture> : public events::IEvent, public impl::ResourceEvent<ecore::Texture>
	{
		public:
			EVENT_TYPE_DECL(TextureLoadedEvent)
			ResourceLoadedEvent(ecore::Texture* texture) : ResourceEvent(texture) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Level> : public events::IEvent, public impl::ResourceEvent<ecore::Level>
	{
		public:
			EVENT_TYPE_DECL(LevelCreatedEvent)
			ResourceCreatedEvent(ecore::Level* level) : ResourceEvent(level) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::Level> : public events::IEvent, public impl::ResourceEvent<ecore::Level>
	{
		public:
			EVENT_TYPE_DECL(LevelLoadedEvent)
			ResourceLoadedEvent(ecore::Level* level) : ResourceEvent(level) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Material> : public events::IEvent, public impl::ResourceEvent<ecore::Material>
	{
		public:
			EVENT_TYPE_DECL(MaterialCreatedEvent)
			ResourceCreatedEvent(ecore::Material* material) : ResourceEvent(material) { }
	};

	template<>
	class ResourceRecreatedEvent<ecore::Material> : public events::IEvent, public impl::ResourceEvent<ecore::Material>
	{
		public:
			EVENT_TYPE_DECL(MaterialRecreatedEvent)
			ResourceRecreatedEvent(ecore::Material* material) : ResourceEvent(material) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::Material> : public events::IEvent, public impl::ResourceEvent<ecore::Material>
	{
		public:
			EVENT_TYPE_DECL(MaterialLoadedEvent)
			ResourceLoadedEvent(ecore::Material* material) : ResourceEvent(material) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::MaterialTemplate> : public events::IEvent, public impl::ResourceEvent<ecore::MaterialTemplate>
	{
		public:
			EVENT_TYPE_DECL(MaterialTemplateCreatedEvent)
			ResourceCreatedEvent(ecore::MaterialTemplate* materialTemplate) : ResourceEvent(materialTemplate) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::MaterialTemplate> : public events::IEvent, public impl::ResourceEvent<ecore::MaterialTemplate>
	{
		public:
			EVENT_TYPE_DECL(MaterialTemplateLoadedEvent)
			ResourceLoadedEvent(ecore::MaterialTemplate* materialTemplate) : ResourceEvent(materialTemplate) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Script> : public events::IEvent, public impl::ResourceEvent<ecore::Script>
	{
		public:
			EVENT_TYPE_DECL(ScriptCreatedEvent)
			ResourceCreatedEvent(ecore::Script* script) : ResourceEvent(script) { }
	};

	template<>
	class ResourceLoadedEvent<ecore::Script> : public events::IEvent, public impl::ResourceEvent<ecore::Script>
	{
		public:
			EVENT_TYPE_DECL(ScriptLoadedEvent)
			ResourceLoadedEvent(ecore::Script* script) : ResourceEvent(script) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Video> : public events::IEvent, public impl::ResourceEvent<ecore::Video>
	{
		public:
			EVENT_TYPE_DECL(VideoCreatedEvent)
			ResourceCreatedEvent(ecore::Video* video) : ResourceEvent(video) { }
	};
	
	template<>
	class ResourceRecreatedEvent<ecore::Video> : public events::IEvent, public impl::ResourceEvent<ecore::Video>
	{
		public:
			EVENT_TYPE_DECL(VideoRecreatedEvent)
			ResourceRecreatedEvent(ecore::Video* video) : ResourceEvent(video) { }
	};
		
	template<>
	class ResourceLoadedEvent<ecore::Video> : public events::IEvent, public impl::ResourceEvent<ecore::Video>
	{
		public:
			EVENT_TYPE_DECL(VideoLoadedEvent)
			ResourceLoadedEvent(ecore::Video* video) : ResourceEvent(video) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Font> : public events::IEvent, public impl::ResourceEvent<ecore::Font>
	{
		public:
			EVENT_TYPE_DECL(FontCreatedEvent)
			ResourceCreatedEvent(ecore::Font* font) : ResourceEvent(font) { }
	};
	
	template<>
	class ResourceRecreatedEvent<ecore::Font> : public events::IEvent, public impl::ResourceEvent<ecore::Font>
	{
		public:
			EVENT_TYPE_DECL(FontRecreatedEvent)
			ResourceRecreatedEvent(ecore::Font* font) : ResourceEvent(font) { }
	};
	
	
	template<>
	class ResourceLoadedEvent<ecore::Font> : public events::IEvent, public impl::ResourceEvent<ecore::Font>
	{
		public:
			EVENT_TYPE_DECL(FontLoadedEvent)
			ResourceLoadedEvent(ecore::Font* font) : ResourceEvent(font) { }
	};

	template<>
	class ResourceCreatedEvent<ecore::Sound> : public events::IEvent, public impl::ResourceEvent<ecore::Sound>
	{
		public:
			EVENT_TYPE_DECL(SoundCreatedEvent)
			ResourceCreatedEvent(ecore::Sound* sound) : ResourceEvent(sound) { }
	};
	
	template<>
	class ResourceRecreatedEvent<ecore::Sound> : public events::IEvent, public impl::ResourceEvent<ecore::Sound>
	{
		public:
			EVENT_TYPE_DECL(SoundRecreatedEvent)
			ResourceRecreatedEvent(ecore::Sound* sound) : ResourceEvent(sound) { }
	};
	
	template<>
	class ResourceLoadedEvent<ecore::Sound> : public events::IEvent, public impl::ResourceEvent<ecore::Sound>
	{
		public:
			EVENT_TYPE_DECL(SoundLoadedEvent)
			ResourceLoadedEvent(ecore::Sound* sound) : ResourceEvent(sound) { }
	};
}
