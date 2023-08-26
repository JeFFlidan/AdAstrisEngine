#pragma once

#include "resource_events_impl.h"
#include "events/event.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"

namespace ad_astris::resource
{
	class StaticModelLoadedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ModelLoadedEvent)
			StaticModelLoadedEvent(ecore::StaticModel* staticModel) : _staticModel(staticModel) { }

			ecore::StaticModel* get_model()
			{
				return _staticModel;
			}

		private:
			ecore::StaticModel* _staticModel{ nullptr };
	};

	class Texture2DLoadedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(Texture2DLoadedEvent)
			Texture2DLoadedEvent(ecore::Texture2D* texture) : _texture(texture) { }

			ecore::Texture2D* get_texture()
			{
				return _texture;
			}

		private:
			ecore::Texture2D* _texture{ nullptr };
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
}
