#pragma once

#include "engine_core/uuid.h"
#include "profiler/logger.h"
#include "type_info_table.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <mutex>

namespace ad_astris::ecs
{
	class Archetype;
	class EntityManager;
	class ArchetypeCreationContext;
	class ArchetypeExtensionContext;
	class EntityCreationContext;
	
	namespace constants
	{
		constexpr uint32_t MAX_COMPONENT_SIZE = 128;
		constexpr uint32_t MAX_COMPONENT_COUNT = 15;
		constexpr uint32_t MAX_CHUNK_SIZE = MAX_COMPONENT_SIZE * MAX_COMPONENT_COUNT;
	}
	
	class Entity
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

	class IComponent
	{
		public:
			virtual ~IComponent() { }
		
			virtual uint32_t get_type_id() = 0;
			virtual void* get_raw_memory() = 0;
			virtual uint16_t get_structure_size() = 0;
	};

	class UntypedComponent : public IComponent
	{
		friend Archetype;
		friend EntityManager;
		friend ArchetypeCreationContext;
		friend ArchetypeExtensionContext;
		friend EntityCreationContext;
		
		public:
			UntypedComponent() = default;
			
			UntypedComponent(void* memory, uint32_t size, uint32_t id) : _memory(memory), _size(size), _id(id)
			{
				
			}

			virtual ~UntypedComponent() override { }

			// ====== Begin IComponent interface ======
			
			virtual uint32_t get_type_id() override
			{
				return _id;
			}
			
			virtual void* get_raw_memory() override
			{
				return _memory;
			}
			
			virtual uint16_t get_structure_size() override
			{
				return _size;
			}

			// ====== End IComponent interface ======

		protected:
			void* _memory{ nullptr };
			uint32_t _size{ 0 };
			uint32_t _id{ 0 };

			// Need this for serialization. I can clear data using destructor but I want to do it explicitly
			virtual void destroy_component_value() { } 
	};

	template<typename T>
	class Component : public IComponent
	{
		public:
			Component() = default;
			Component(T& component) : _component(component) { }
		
			virtual uint32_t get_type_id() { return TYPE_INFO_TABLE->get_component_id<T>(); }
			virtual void* get_raw_memory() { return &_component; }
			virtual uint16_t get_structure_size() { return TYPE_INFO_TABLE->get_component_size<T>(); }

		private:
			T _component;
	};
	
	class EntityCreationContext
	{
		friend Archetype;
		friend EntityManager;
		
		public:
			EntityCreationContext()
			{
				_componentIDs.reserve(constants::MAX_COMPONENT_COUNT);
			}
		
			~EntityCreationContext()
			{
				
			}

			template<typename T>
			void add_tag()
			{
				uint32_t id = TYPE_INFO_TABLE->get_tag_id<T>();
				if (!check_tag(id))
					return;

				_tagIDs.push_back(id);
			}
		
			template<typename T>
			void add_component(T& value)
			{
				uint32_t typeId = TYPE_INFO_TABLE->get_component_id<T>();
				
				if (!check_component(typeId, sizeof(T)))
					return;
				
				_allComponentsSize += TYPE_INFO_TABLE->get_component_size<T>();
				std::unique_ptr<IComponent> component(new Component<T>(value));
				_sizeByTypeID[typeId] = component->get_structure_size();
				_componentsMap[typeId] = std::move(component);
				_componentIDs.push_back(typeId);
			}

			template<typename T, typename ...ARGS>
			void add_component(ARGS&&... args)
			{
				uint32_t typeId = TYPE_INFO_TABLE->get_component_id<T>();
				
				if (!check_component(typeId, sizeof(T)))
					return;
				
				_allComponentsSize += TYPE_INFO_TABLE->get_component_size<T>();
				
				std::unique_ptr<IComponent> component(new Component<T>( T{std::forward<ARGS>(args)...} ));
				_sizeByTypeID[typeId] = component->get_structure_size();
				_componentsMap[typeId] = std::move(component);
				_componentIDs.push_back(typeId);
			}

			template<typename ...ARGS>
			void add_components(ARGS&&... args)
			{
				((add_component(args)), ...);
			}

			template<typename T>
			T get_component()
			{
				auto it = _componentsMap.find(TYPE_INFO_TABLE->get_component_id<T>());
				if (it == _componentsMap.end())
					LOG_ERROR("EntityCreationContext::get_component(): Creation context doesn't contain component {}", get_type_name<T>())
				return *static_cast<T*>(it->second->get_raw_memory());
			}

			template<typename T>
			void set_component(T& value)
			{
				uint32_t id = TYPE_INFO_TABLE->get_component_id<T>();
				
				if (!check_component(id, sizeof(T)))
					return;

				IComponent* component = _componentsMap.find(id)->second.get();
				*component = Component<T>(value);
			}

			template<typename T, typename ...ARGS>
			void set_component(ARGS&&... args)
			{
				uint32_t id = TYPE_INFO_TABLE->get_component_id<T>();
				
				if (!check_component(id, sizeof(T)))
					return;

				//Component<T>* component = dynamic_cast<Component<T*>>(_componentsMap.find(id))->second;
				IComponent* component = _componentsMap.find(id)->second.get();
				*component = Component<T>(T{std::forward<ARGS>(args)...});
			}

			template<typename ...ARGS>
			void set_components(ARGS&&... args)
			{
				((set_component(args)), ...);
			}

			template<typename T>
			void remove_component()
			{
				uint32_t typeID = TYPE_INFO_TABLE->get_component_id<T>();
				
				if (!is_component_added(typeID))
				{
					return;
				}
			
				auto it = _componentsMap.find(typeID);
				_componentIDs.erase(std::find(_componentIDs.begin(), _componentIDs.end(), typeID));
				_componentsMap.erase(it);
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
			uint32_t _allComponentsSize{ 0 };
			std::vector<uint32_t> _componentIDs;
			std::unordered_map<uint32_t, std::unique_ptr<IComponent>> _componentsMap;
			std::unordered_map<uint32_t, uint16_t> _sizeByTypeID;
			std::vector<uint32_t> _tagIDs;

			bool check_component(uint32_t id, uint32_t size)
			{
				if (_componentsMap.find(id) != _componentsMap.end())
				{
					LOG_ERROR("EntityCreationContext::add_component(): Component of this type was added")
					return false;
				}

				return true;
			}

			bool check_tag(uint32_t id)
			{
				if (std::find(_tagIDs.begin(), _tagIDs.end(), id) != _tagIDs.end())
				{
					LOG_ERROR("EntityCreationContext::add_tag(): Tag of this type was added")
					return false;
				}

				return true;
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

