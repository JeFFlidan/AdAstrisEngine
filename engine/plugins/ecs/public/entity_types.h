#pragma once

#include "api.h"
#include "utils.h"
#include "engine_core/uuid.h"
#include "profiler/logger.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

namespace ad_astris::ecs
{
	class Archetype;
	class EntitySystem;
	class ArchetypeCreationContext;
	class ArchetypeExtensionContext;
	class EntityCreationContext;
	
	namespace constants
	{
		constexpr uint32_t MAX_COMPONENT_SIZE = 128;
		constexpr uint32_t MAX_COMPONENT_COUNT = 15;
		constexpr uint32_t MAX_CHUNK_SIZE = MAX_COMPONENT_SIZE * MAX_COMPONENT_COUNT;
	}
	
	class ECS_API Entity
	{
		public:
			Entity();
			Entity(UUID uuid);
			UUID get_uuid();

			bool operator==(const Entity& other);
			bool operator!=(const Entity& other);
			operator uint64_t() const;
		
		private:
			UUID _uuid;
	};

	class ECS_API IComponent
	{
		public:
			virtual ~IComponent() { }
		
			virtual uint32_t get_type_id() = 0;
			virtual void* get_raw_memory() = 0;
			virtual uint32_t get_structure_size() = 0;
	};

	template<typename T>
	class Component : public IComponent
	{
		friend Archetype;
		friend EntitySystem;
		friend ArchetypeCreationContext;
		friend ArchetypeExtensionContext;
		friend EntityCreationContext;
		
		public:
			Component(void* memory);

			// ====== Begin IComponent interface ======
		
			virtual uint32_t get_type_id() override;
			virtual void* get_raw_memory() override;
			virtual uint32_t get_structure_size() override;

			// ====== End IComponent interface ======
		
			T* get_memory();

		private:
			static const uint32_t _typeId;
			void* _memory{ nullptr };
			uint32_t _structureSize{ 0 };
	};

	template<typename T>
	const uint32_t Component<T>::_typeId = IdGenerator<IComponent>::generate_id<T>();

	class ECS_API EntityCreationContext
	{
		friend Archetype;
		friend EntitySystem;
		
		public:
			EntityCreationContext()
			{
				_componetsId.resize(constants::MAX_COMPONENT_COUNT);
			}
		
			~EntityCreationContext()
			{
				if (_componentsData)
					delete[] _componentsData;

				for (auto& it : _componentsMap)
					delete it.second;
			}
		
			template<typename T>
			void add_component(T& value)
			{
				if (!check(Component<T>::_typeId, sizeof(T)))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}

				uint8_t* dataPtr = get_ptr_for_data();
				memcpy(dataPtr, &value, sizeof(T));

				IComponent* tempComponent = new Component<T>(dataPtr);
				_componentsMap[tempComponent->get_type_id()] = tempComponent;
				_componetsId.push_back(tempComponent->get_type_id());
			}

			template<typename T>
			void add_component(Component<T>& component)
			{
				if (!check(component.get_type_id(), sizeof(T)))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}

				uint8_t* dataPtr = get_ptr_for_data();
				memcpy(dataPtr, component.get_memory(), sizeof(T));

				IComponent* tempComponent = new Component<T>(dataPtr);
				_componentsMap[tempComponent->get_type_id()] = tempComponent;
				_componetsId.push_back(tempComponent->get_type_id());
			}

			template<typename T, typename ...ARGS>
			void add_component(ARGS&&... args)
			{
				if (!check(Component<T>::_typeId, sizeof(T)))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}

				uint8_t* dataPtr = get_ptr_for_data();
				new(dataPtr) T(std::forward<ARGS>(args)...);
				IComponent* tempComponent = new Component<T>(dataPtr);
				
				_componentsMap[tempComponent->get_type_id()] = tempComponent;
				_componetsId.push_back(tempComponent->get_type_id());
			}

			template<typename ...ARGS>
			void add_components(ARGS&&... args)
			{
				((add_component(args)), ...);
			}

			template<typename T>
			void set_component(T& value)
			{
				uint32_t id = Component<T>::_typeId;
				
				if (!check(id, sizeof(T)))
					return;

				Component<T>* component = dynamic_cast<Component<T>*>(_componentsMap.find(id)->second);
				memcpy(component->get_memory(), &value, sizeof(T));
			}

			template<typename T>
			void set_component(Component<T>& component)
			{
				uint32_t id = component.get_type_id();
				
				if (!check(id, sizeof(T)))
					return;

				Component<T>* tempComponent = dynamic_cast<Component<T*>>(_componentsMap.find(id)->second);
				memcpy(tempComponent->get_memory(), component.get_memory(), sizeof(T));
			}

			template<typename T, typename ...ARGS>
			void set_component(ARGS&&... args)
			{
				uint32_t id = Component<T>::_typeId;
				
				if (!check(id, sizeof(T)))
					return;

				Component<T>* component = dynamic_cast<Component<T*>>(_componentsMap.find(id))->second;
				new(component->get_memory()) T(std::forward<ARGS>(args)...);
			}

			template<typename ...ARGS>
			void set_components(ARGS&&... args)
			{
				((set_component(args)), ...);
			}

			template<typename T>
			void remove_component()
			{
				uint32_t typeID = Component<T>::_typeId;
				
				if (!is_component_added(typeID))
				{
					return;
				}
			
				auto it = _componentsMap.find(typeID);
				_componetsId.erase(std::find(_componetsId.begin(), _componetsId.end(), typeID));
				_removedData.push_back(static_cast<uint8_t*>(it->second->get_raw_memory()));
				_componentsMap.erase(it);
				delete it->second;
			}

			template<typename ...ARGS>
			void remove_components()
			{
				((remove_component<ARGS>()), ...);
			}

			bool is_component_added(uint32_t componentTypeId)
			{
				if (_componentsMap.find(componentTypeId) != _componentsMap.end())
					return true;
				LOG_ERROR("EntityCreationContext::remove_component(): No component with this type")
				return false;
			}

		private:
			std::vector<uint32_t> _componetsId;
			std::unordered_map<uint32_t, IComponent*> _componentsMap;
			uint8_t* _componentsData{ nullptr };
			std::vector<uint8_t*> _removedData;

			bool check(uint32_t id, uint32_t size)
			{
				if (_componentsMap.find(id) != _componentsMap.end())
				{
					LOG_ERROR("EntityCreationContext::add_component(): Component of this type was added")
					return false;
				}

				if (size > constants::MAX_COMPONENT_SIZE)
					return false;

				return true;
			}

			uint8_t* get_ptr_for_data()
			{
				uint8_t* dataPtr{ nullptr };
				if (_removedData.empty())
				{
					uint32_t offset = constants::MAX_COMPONENT_SIZE * _componentsMap.size();
					dataPtr = _componentsData + offset;
				}
				else
				{
					dataPtr = _removedData.back();
					_removedData.pop_back();
				}

				return dataPtr;
			}
	};
}

namespace std
{
	template<>
	struct hash<ad_astris::ecs::Entity>
	{
		std::size_t operator()(const ad_astris::ecs::Entity& entity) const
		{
			return hash<uint64_t>()((uint64_t)entity);
		}
	};
}

