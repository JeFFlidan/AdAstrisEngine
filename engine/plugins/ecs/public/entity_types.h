#pragma once

#include "api.h"
#include "engine_core/uuid.h"
#include "profiler/logger.h"
#include "core/reflection.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <mutex>

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

	class ECS_API ComponentTypeIDTable
	{
		public:
			static ComponentTypeIDTable* get_instance()
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (!_instance)
					_instance = new ComponentTypeIDTable();
				return _instance;
			}
		
			template<typename T>
			bool check_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_table.find(typeName) != _table.end())
					return true;
				return false;
			}

			template<typename T>
			uint32_t get_type_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;

				return it->second.id;
			}

			uint32_t get_type_id(std::string& typeName)
			{
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;

				return it->second.id;
			}

			template<typename T>
			uint32_t get_type_size()
			{
				std::string typeName = get_type_name<T>();
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;

				return it->second.structureSize;
			}

			uint32_t get_type_size(std::string& typeName)
			{
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;

				return it->second.structureSize;
			}

			template<typename T>
			void add_component_info()
			{
				std::lock_guard<std::mutex> lock(_mutex);
				std::string typeName = get_type_name<T>();
				if (_table.find(typeName) != _table.end())
					return;

				ComponentInfo info;
				info.id = ++_idGenerator;
				info.structureSize = sizeof(T);
				_table[typeName] = info;
			}
		
		private:
			ComponentTypeIDTable() {}
			~ComponentTypeIDTable() {}
		
			struct ComponentInfo
			{
				uint32_t structureSize;
				uint32_t id;
			};

			static ComponentTypeIDTable* _instance;
			static std::mutex _mutex;

			static std::unordered_map<std::string, ComponentInfo> _table;
			static uint32_t _idGenerator;
	};

	// Returns the instance of the singleton ComponentTypeIDTable.
	inline ComponentTypeIDTable* get_type_id_table()
	{
		return ComponentTypeIDTable::get_instance();
	}
	
	class ECS_API IComponent
	{
		public:
			virtual ~IComponent() { }
		
			virtual uint32_t get_type_id() = 0;
			virtual void* get_raw_memory() = 0;
			virtual uint32_t get_structure_size() = 0;
	};

	class ECS_API UntypedComponent : public IComponent
	{
		friend Archetype;
		friend EntitySystem;
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
			
			virtual uint32_t get_structure_size() override
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
	
	class ECS_API EntityCreationContext
	{
		friend Archetype;
		friend EntitySystem;
		
		public:
			EntityCreationContext()
			{
				_componentsId.reserve(constants::MAX_COMPONENT_COUNT);
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
				uint32_t typeId = get_type_id_table()->get_type_id<T>();
				
				if (!check(typeId, sizeof(T)))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}
				
				_allComponentsSize += get_type_id_table()->get_type_size<T>();

				uint8_t* dataPtr = get_ptr_for_data();
				memcpy(dataPtr, &value, sizeof(T));

				uint32_t size = get_type_id_table()->get_type_size<T>();
				IComponent* tempComponent = new UntypedComponent(dataPtr, size, typeId);
				_componentsMap[typeId] = tempComponent;
				_typeIdToSize[typeId] = size;
				_componentsId.push_back(typeId);
			}

			template<typename T, typename ...ARGS>
			void add_component(ARGS&&... args)
			{
				uint32_t typeId = get_type_id_table()->get_type_id<T>();
				
				if (!check(typeId, sizeof(T)))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}
				
				_allComponentsSize += get_type_id_table()->get_type_size<T>();

				uint8_t* dataPtr = get_ptr_for_data();
				new(dataPtr) T(std::forward<ARGS>(args)...);

				uint32_t size = get_type_id_table()->get_type_size<T>();
				IComponent* tempComponent = new UntypedComponent(dataPtr, size, typeId);
				
				_componentsMap[typeId] = tempComponent;
				_componentsId.push_back(typeId);
				_typeIdToSize[typeId] = size;
			}

			void add_component(IComponent* component)
			{
				uint32_t typeId = component->get_type_id();
				uint32_t componentSize = component->get_structure_size();
				if (!check(typeId, componentSize))
					return;
				
				if (!_componentsData)
				{
					_componentsData = new uint8_t[constants::MAX_CHUNK_SIZE];
				}

				_allComponentsSize += componentSize;
				
				uint8_t* dataPtr = get_ptr_for_data();
				memcpy(dataPtr, component->get_raw_memory(), componentSize);
				IComponent* newComponent = new UntypedComponent(dataPtr, componentSize, component->get_type_id());
				_componentsMap[typeId] = newComponent;
				_componentsId.push_back(typeId);
				_typeIdToSize[typeId] = componentSize;
			}

			template<typename ...ARGS>
			void add_components(ARGS&&... args)
			{
				((add_component(args)), ...);
			}

			template<typename T>
			void set_component(T& value)
			{
				uint32_t id = get_type_id_table()->get_type_id<T>();
				
				if (!check(id, sizeof(T)))
					return;

				IComponent* component = _componentsMap.find(id)->second; 
				memcpy(component->get_raw_memory(), &value, sizeof(T));
			}

			template<typename T, typename ...ARGS>
			void set_component(ARGS&&... args)
			{
				uint32_t id = get_type_id_table()->get_type_id<T>();
				
				if (!check(id, sizeof(T)))
					return;

				//Component<T>* component = dynamic_cast<Component<T*>>(_componentsMap.find(id))->second;
				IComponent* component = _componentsMap.find(id)->second;
				new(component->get_raw_memory()) T(std::forward<ARGS>(args)...);
			}

			template<typename ...ARGS>
			void set_components(ARGS&&... args)
			{
				((set_component(args)), ...);
			}

			template<typename T>
			void remove_component()
			{
				uint32_t typeID = get_type_id_table()->get_type_id<T>();
				
				if (!is_component_added(typeID))
				{
					return;
				}
			
				auto it = _componentsMap.find(typeID);
				_componentsId.erase(std::find(_componentsId.begin(), _componentsId.end(), typeID));
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
			uint32_t _allComponentsSize{ 0 };
			std::vector<uint32_t> _componentsId;
			std::unordered_map<uint32_t, IComponent*> _componentsMap;
			std::unordered_map<uint32_t, uint16_t> _typeIdToSize;
		
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

