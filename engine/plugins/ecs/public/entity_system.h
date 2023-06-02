#pragma once

#include "api.h"
#include "entity_types.h"
#include "archetype_types.h"
#include "archetype.h"
#include "core/tuple.h"
#include <vector>
#include <unordered_map>

#include "factories.h"

namespace ad_astris::ecs
{
	class ECS_API EntitySystem
	{
		public:
			/**
			 * 
			 */
			ArchetypeHandle create_archetype(ArchetypeCreationContext& context);

			/**
			 * 
			 */
			ArchetypeHandle create_archetype(ArchetypeExtensionContext& context);

			/**
			 * 
			 */
			ArchetypeHandle create_archetype(ArchetypeReductionContext context);
		
			/**
			 * 
			 */
			ArchetypeHandle get_entity_archetype(Entity& entity);
		
			/** Creates an empty entity without archetype. Can't be used in an engine
			 * until setting up an archetype. 
			 * @return new entity without archetype and related components*/ 
			Entity create_entity();
		
			/** 
			 * 
			 */
			Entity create_entity(ArchetypeHandle& archetype);

			/**
			 * 
			 */
			Entity create_entity(EntityCreationContext& entityContext, UUID uuid = 0);

			/** UUID and json with all entity components are taken from the level file.
			 * 
			 */
			Entity build_entity_from_json(UUID& uuid, std::string& json);

			/**
			 * @param entity should have a valid UUID
			 * @param levelJson is a main level json consisting entities and value of there components
			 */
			void build_components_json_from_entity(Entity& entity, nlohmann::json& levelJson);

			/**
			 * 
			 */
			void destroy_entity(Entity& entity);

			/**
			 * 
			 */
			template<typename ...ARGS>
			void add_components_to_entity(Entity& entity)
			{
				EntityInArchetypeInfo oldEntityInArchetype = _entityToItsInfoInArchetype[entity];
				ArchetypeExtensionContext extensionContext(oldEntityInArchetype.archetypeId);
				extensionContext.add_components_id<ARGS...>();
				ArchetypeHandle newArchetypeHandle = create_archetype(extensionContext);

				Archetype& oldArchetype = _archetypes[oldEntityInArchetype.archetypeId];
				oldArchetype.destroy_entity(entity, oldEntityInArchetype.column);
				
				Archetype& newArchetype = _archetypes[newArchetypeHandle.get_id()];
				uint32_t column = newArchetype.add_entity(entity);

				EntityInArchetypeInfo newEntityInArchetype;
				newEntityInArchetype.column = column;
				newEntityInArchetype.archetypeId = newArchetypeHandle.get_id();

				_entityToItsInfoInArchetype[entity] = newEntityInArchetype;
			}

			/**
			 * 
			 */
			template<typename ...ARGS>
			void remove_components_from_entity(Entity& entity)
			{
				EntityInArchetypeInfo oldEntityInArchetype = _entityToItsInfoInArchetype[entity];
				ArchetypeReductionContext reductionContext(oldEntityInArchetype.archetypeId);
				reductionContext.add_components_id<ARGS...>();
				ArchetypeHandle newArchetypeHandle = create_archetype(reductionContext);

				Archetype& oldArchetype = _archetypes[oldEntityInArchetype.archetypeId];
				oldArchetype.destroy_entity(entity, oldEntityInArchetype.column);
				
				Archetype& newArchetype = _archetypes[newArchetypeHandle.get_id()];
				uint32_t column = newArchetype.add_entity(entity);

				EntityInArchetypeInfo newEntityInArchetype;
				newEntityInArchetype.column = column;
				newEntityInArchetype.archetypeId = newArchetypeHandle.get_id();

				_entityToItsInfoInArchetype[entity] = newEntityInArchetype;
			}

			/**
			 * 
			 */
			template<typename T>
			void set_entity_component(Entity& entity, T& value)
			{
				set_up_component_common(entity, &value);
			}

			/**
			 * 
			 */
			template<typename T, typename ...ARGS>
			void set_entity_component(Entity& entity, ARGS&&... args)
			{
				T value{ std::forward<ARGS>(args)... };
				set_up_component_common(entity, &value);
			}

			/**
			 * 
			 */
			template<typename T>
			T* get_entity_component(Entity& entity)
			{
				EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
				Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
				T* component = archetype.get_entity_component<T>(entity, entityInArchetype.column);
				return component;
			}

			template<typename ...ARGS>
			Tuple<ARGS*...> get_entity_components(Entity& entity)
			{
				EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
				Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
				Tuple<ARGS*...> components = archetype.get_entity_components<ARGS...>(entity, entityInArchetype.column);
				return components;
			}

			template<typename ...ARGS>
			std::vector<Tuple<ARGS*...>> get_component_arrays()
			{
				// TODO
			}

			// Should be used to build component from Level file. After building component, the best
			// solution is add this component to the Entity.
			template<typename ...ARGS>
			UntypedComponent build_component(std::string& typeName, ARGS&... args)
			{
				uint32_t typeId = get_type_id_table()->get_type_id(typeName);
				//void* value = FactoriesTable::get_instance()->get_factory(typeId)->build();
				
			}

		private:
			struct EntityInArchetypeInfo
			{
				uint32_t archetypeId;
				uint32_t column;
			};
		
			std::vector<Archetype> _archetypes;
			std::unordered_map<size_t, uint32_t> _componentsHashToArchetypeId;
			std::unordered_map<Entity, EntityInArchetypeInfo> _entityToItsInfoInArchetype;

			template<typename T>
			void set_up_component_common(Entity& entity, T* componentValue)
			{
				UntypedComponent component(componentValue, get_type_id_table()->get_type_size<T>(), get_type_id_table()->get_type_id<T>());
				EntityInArchetypeInfo& entityInArchetype = _entityToItsInfoInArchetype[entity];
				Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
				archetype.set_component(entity, entityInArchetype.column, &component);
			}
	};
}
