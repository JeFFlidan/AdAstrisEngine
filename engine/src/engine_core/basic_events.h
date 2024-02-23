#pragma once

#include "enums.h"
#include "events/event.h"
#include "ecs/entity_manager.h"

namespace ad_astris::ecore
{
	class LocalTransformUpdateEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(LocalTransformUpdateEvent)

			LocalTransformUpdateEvent(const ecs::Entity& entity) : _entity(entity) { }

			bool need_location_update() { return _needLocationUpdate; }
			XMFLOAT3 get_location() { return _location; }
			void set_location(XMFLOAT3 location)
			{
				_location = location;
				_needLocationUpdate = true;
			}

			bool need_rotation_update() { return _needRotationUpdate; }
			XMFLOAT3 get_rotation() { return _rotation; }
			void set_rotation(XMFLOAT3 rotation)
			{
				_rotation = rotation;
				_needRotationUpdate = true;
			}

			bool need_scale_update() { return _needScaleUpdate; }
			XMFLOAT3 get_scale() { return _scale; }
			void set_scale(XMFLOAT3 scale)
			{
				_scale = scale;
				_needScaleUpdate = true;
			}

			ecs::Entity get_entity() { return _entity; }

		private:
			ecs::Entity _entity;
			XMFLOAT3 _rotation;
			XMFLOAT3 _location;
			XMFLOAT3 _scale;
			bool _needLocationUpdate{ false };
			bool _needRotationUpdate{ false };
			bool _needScaleUpdate{ false };
	};

	class EntityCreatedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(EntityCreatedEvent)
			EntityCreatedEvent(ecs::Entity& entity, ecs::EntityManager* entityManager) : _entity(entity), _entityManager(entityManager) { }

			ecs::Entity get_entity() { return _entity; }
			ecs::EntityManager* get_entity_manager() { return _entityManager; }

		private:
			ecs::Entity _entity;
			ecs::EntityManager* _entityManager{ nullptr };
	};

	class CameraSetEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(CameraSetEvent)
			CameraSetEvent(ecs::Entity camera, CameraIndex cameraIndex) : _camera(camera), _cameraIndex(cameraIndex) { }

			ecs::Entity get_camera() const { return _camera; }
			CameraIndex get_camera_index() const { return _cameraIndex; }

		private:
			ecs::Entity _camera;
			CameraIndex _cameraIndex;
	};
}
