#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/shader.h"

namespace ad_astris::resource
{
	class StaticModelCreatedEvent : public events::IEvent, public impl::StaticModelEvent
	{
		public:
			EVENT_TYPE_DECL(StaticModelCreatedEvent)
			StaticModelCreatedEvent(ecore::StaticModel* model) : StaticModelEvent(model) { }
	};
	
	class StaticModelLoadedEvent : public events::IEvent, public impl::StaticModelEvent
	{
		public:
			EVENT_TYPE_DECL(StaticModelLoadedEvent)
			StaticModelLoadedEvent(ecore::StaticModel* model) : StaticModelEvent(model) { }
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

	class MaterialTemplateLoadedEvent : public events::IEvent, public impl::MaterialTemplateEvent
	{
		public:
			EVENT_TYPE_DECL(MaterialTemplateLoadedEvent)
			MaterialTemplateLoadedEvent(ecore::MaterialTemplate* materialTemplate) : MaterialTemplateEvent(materialTemplate) { }
	};

	class MaterialTemplateCreatedEvent : public events::IEvent, public impl::MaterialTemplateEvent
	{
		public:
			EVENT_TYPE_DECL(MaterialTemplateCreatedEvent)
			MaterialTemplateCreatedEvent(ecore::MaterialTemplate* materialTemplate) : MaterialTemplateEvent(materialTemplate) { }
	};

	class ShaderLoadedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ShaderLoadedEvent)
			ShaderLoadedEvent(ecore::Shader* shader) : _shader(shader) { }

			ecore::ShaderHandle get_shader_handle()
			{
				return _shader;
			}

		private:
			ecore::Shader* _shader{ nullptr };
	};
}
