#pragma once

#include "entity_types.h"
#include "archetype_types.h"
#include "archetype.h"
#include <vector>
#include <unordered_map>

namespace ad_astris::ecs
{
	class EntityManager
	{
		friend class SystemManager;
		
		public:
			/** Creates a new archetype based on the ArchetypeCreationContext. If the archetype exists with the same
			 * components and tags, this method will return ArchetypeHandle for that existing archetype.
			 * @param context must be valid context with configuration of required components or tags.
			 * If you don't configure components and tags, context will be invalid.
			 */
			ArchetypeHandle create_archetype(ArchetypeCreationContext& context);

			/**  Creates a new archetype based on the ArchetypeCreationContext. If the archetype exists with the same
			 * components and tags, this method will return ArchetypeHandle for that existing archetype.
			 * @param context must be a valid extension context that is based on the existing archetype and
			 * components or tags that you want to add. If context consists of invalid ArchetypeHandle, it will
			 * cause an error.
			 */
			ArchetypeHandle create_archetype(ArchetypeExtensionContext& context);

			/**
			 * 
			 */
			ArchetypeHandle create_archetype(ArchetypeReductionContext context);
		
			/** Returns archetype that stores all components of the selected entity.
			 * @param entity must be a valid entity
			 */
			ArchetypeHandle get_entity_archetype(Entity& entity);
		
			/** Creates an empty entity without archetype. Can't be used in an engine
			 * until setting up an archetype. 
			 * @return new entity without archetype and related components*/ 
			Entity create_entity();
		
			/** Creates a new entity and dedicate free space in the chosen Archetype.
			 * @param archetype must be a valid archetype handle. You can get it after creating a new archetype
			 * or calling get_entity_archetype method
			 */
			Entity create_entity(ArchetypeHandle& archetype);

			/** Creates a new entity and configures all required components and tags.
			 * @param entityContext must contain all necessary components and there values. Also, you can define
			 * tags
			 */
			Entity create_entity(EntityCreationContext& entityContext, UUID uuid = 0);

			/** Builds one entity with all components and tags based on the info from .aalevel file
			 * @param uuid must be a valid uuid that was taken from .aalevel file
			 * @param json fully describes one entity. Must be taken from .aalevel file
			 */
			Entity build_entity_from_json(UUID& uuid, std::string& json);

			/** Serializes all entity components and tags to json.
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
				// TODO should improve extension context
				EntityInArchetypeInfo oldEntityInArchetype = _entityToItsInfoInArchetype[entity];
				ArchetypeExtensionContext extensionContext(oldEntityInArchetype.archetypeId);
				extensionContext.add_components<ARGS...>();
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

			template<typename ...ARGS>
			void add_tags_to_entity(Entity& entity)
			{
				
			}

			/**
			 * 
			 */
			template<typename ...ARGS>
			void remove_components_from_entity(Entity& entity)
			{
				// TODO Should improve reduction context
				EntityInArchetypeInfo oldEntityInArchetype = _entityToItsInfoInArchetype[entity];
				ArchetypeReductionContext reductionContext(oldEntityInArchetype.archetypeId);
				reductionContext.add_components<ARGS...>();
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

			/** TODO MAKE THREAD SAFE
			 * 
			 */
			template<typename T>
			void set_entity_component(Entity& entity, T& value)
			{
				set_up_component_common(entity, &value);
			}

			/**  TODO MAKE THREAD SAFE
			 * 
			 */
			template<typename T, typename ...ARGS>
			void set_entity_component(Entity& entity, ARGS&&... args)
			{
				T value{ std::forward<ARGS>(args)... };
				set_up_component_common(entity, &value);
			}

			/** Only for debug
			 * 
			 */
			template<typename T>
			T get_entity_component(Entity& entity)
			{
				EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
				Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
				T* component = archetype.get_entity_component<T>(entity, entityInArchetype.column);
				return *component;
			}

			uint32_t get_archetypes_count()
			{
				return _archetypes.size();
			}

			template<typename ComponentType>
			bool does_entity_have_component(Entity& entity)
			{
				auto it = _entityToItsInfoInArchetype.find(entity);
				Archetype& archetype = _archetypes[it->second.archetypeId];
				return archetype.has_component<ComponentType>();
			}

			template<typename TagType>
			bool does_entity_have_tag(Entity& entity)
			{
				auto it = _entityToItsInfoInArchetype.find(entity);
				Archetype& archetype = _archetypes[it->second.archetypeId];
				return archetype.has_tag<TagType>();
			}
		
		private:
			struct EntityInArchetypeInfo
			{
				uint32_t archetypeId;
				uint32_t column;
			};
		
			std::vector<Archetype> _archetypes;
			std::vector<uint32_t> _lastCreatedArchetypes;
			std::unordered_map<size_t, uint32_t> _componentsHashToArchetypeId;
			std::unordered_map<Entity, EntityInArchetypeInfo> _entityToItsInfoInArchetype;

			template<typename T>
			void set_up_component_common(Entity& entity, T* componentValue)
			{
				UntypedComponent component(componentValue, TypeInfoTable::get_component_size<T>(), TypeInfoTable::get_component_id<T>());
				EntityInArchetypeInfo& entityInArchetype = _entityToItsInfoInArchetype[entity];
				Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
				archetype.set_component(entity, entityInArchetype.column, &component);
			}

			// Returns new vector hash
			size_t merge_type_ids_vectors(
				std::vector<uint32_t>& dstTypeIDs,
				std::vector<uint32_t>& srcTypeIDs,
				std::vector<uint32_t>& contextTypeIDs);

			void copy_vector(
				std::vector<uint32_t>& srcTypeIDs,
				std::vector<uint32_t>& dstTypeIDs);
	};
}