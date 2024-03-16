#pragma once

#include "resource_formats.h"
#include "events/event.h"
#include "engine_core/material/materials.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"

namespace ad_astris::resource::impl
{
	class StaticModelEvent
	{
		public:
			StaticModelEvent(ecore::StaticModel* staticModel) : _model(staticModel) { }

			ResourceAccessor<ecore::StaticModel> get_model_handle()
			{
				return _model;
			}

		private:
			ecore::StaticModel* _model;
	};

	class Texture2DEvent
	{
		public:
			Texture2DEvent(ecore::Texture2D* texture2D) : _texture(texture2D) { }

			ResourceAccessor<ecore::Texture2D> get_texture_handle()
			{
				return _texture;
			}

		private:
			ecore::Texture2D* _texture;
	};

	template<typename T>
	class ResourceEvent
	{
		public:
			ResourceEvent(T* resource) : _resource(resource) { }

			ResourceAccessor<T> get_resource() const { return _resource; }

		private:
			T* _resource{ nullptr };
	};
	
#define DECL_RESOURCE_EVENT(EventName, ResourceType) \
	class EventName : public events::IEvent, public impl::ResourceEvent<ResourceType> \
	{ \
		public:\
			EVENT_TYPE_DECL(EventName)\
			EventName(ResourceType* resource) : ResourceEvent(resource) { } \
	};
}

	
