#pragma once

#include "rhi/resources.h"
#include "events/event.h"
#include <ecs.h>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace ad_astris::renderer::impl
{
	constexpr uint64_t ONE_MEBIBYTE = 1048576;
	constexpr uint64_t DEFAULT_GPU_BUFFER_SIZE = ONE_MEBIBYTE * 32;
	constexpr uint64_t POINT_LIGHT_DEFAULT_COUNT = 32;
	constexpr uint64_t SPOT_LIGHT_DEFAULT_COUNT = 32;
	constexpr uint64_t DIRECTIONAL_LIGHT_DEFAULT_COUNT = 8;

#define IMPLEMENT_SUBMANAGER_EVENT(EventName)															\
	class EventName##UpdatedEvent : public events::IEvent												\
	{																									\
		public:																							\
			EVENT_TYPE_DECL(EventName)																	\
			std::string get_submanager_name() { return #EventName; }									\
	};
	
	struct GPUBuffer
	{
		rhi::Buffer buffer;
		uint32_t offset{ 0 };
	};

	template<typename T>
	class ObjectCPUCollection
	{
		public:
			uint32_t add_object(T& object, ecs::Entity& entity)
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				auto it = _objectIndexByEntity.find(entity);
				if (it != _objectIndexByEntity.end())
				{
					LOG_ERROR("SceneManager::ObjectCPUCollection::add_object(): Entity {} was added earlier", entity.get_uuid())
					return -1;
				}
				uint32_t index = _array.size();
				_objectIndexByEntity[entity] = index;
				_array.push_back(object);
				return index;
			}
					
			T* begin()
			{
				return _array.data();
			}

			uint32_t size()
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				return _array.size();
			}

			T& operator[](ecs::Entity& entity)
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				auto it = _objectIndexByEntity.find(entity);
                if (it == _objectIndexByEntity.end())
                {
                	LOG_FATAL("SceneManager::ObjectCPUCollection::add_object(): Collection doesn't have entity {}", entity.get_uuid())
                }
                return _array[it->second];
			}

			T& operator[](uint32_t index)
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				if (index >= _array.size())
					LOG_FATAL("SceneManager::ObjectCPUCollection::operator[]: Index {} > array size {}", index, _array.size())
				return _array[index];
			}
				
		private:
			std::vector<T> _array;
			std::unordered_map<ecs::Entity, uint32_t> _objectIndexByEntity;
			std::mutex _mutex;
	};
}