#pragma once

#include "api.h"
#include "entity_types.h"
#include "archetype_types.h"
#include "archetype.h"
#include "core/tuple.h"
#include <vector>
#include <unordered_map>

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
			Entity create_entity(EntityCreationContext& entityContext);

			template<typename T>
			void add_component_to_entity(Entity& entity)
			{
				
			}

			/**
			 * 
			 */
			template<typename T>
			void add_component_to_entity(Entity& entity, T& value)
			{
				
			}

			/**
			 * 
			 */
			template<typename T>
			void add_component_to_entity(Entity& entity, Component<T>& component)
			{
				
			}

			/**
			 * 
			 */
			template<typename T, typename ...ARGS>
			void add_component_to_entity(Entity& entity, ARGS&&... args)
			{
				
			}

			/**
			 * 
			 */
			template<typename T>
			void set_entity_component(Entity& entity, T& value)
			{
				Component<T> component(&value);
				set_entity_component(entity, &component);
			}

			/**
			 * 
			 */
			template<typename T>
			void set_entity_component(Entity& entity, Component<T>& component)
			{
				set_up_component_common(entity, &component);
			}

			/**
			 * 
			 */
			template<typename T, typename ...ARGS>
			void set_entity_component(Entity& entity, ARGS&&... args)
			{
				T value{ std::forward<ARGS>(args)... };
				Component<T> component{ &value };
				set_up_component_common(entity, &component);
			}

			/**
			 * 
			 */
			template<typename T>
			T* get_entity_component(Entity& entity)
			{
				Component<T> component;
				// TODO
				return component;
			}

			template<typename ...ARGS>
			std::vector<Tuple<ARGS...>> get_components()
			{
				// TODO
			}
		
		private:
			struct EntityInArchetype
			{
				uint32_t archetypeId;
				uint32_t column;
			};
		
			std::vector<Archetype> _archetypes;
			std::unordered_map<size_t, uint32_t> _componentsHashToArchetypeId;
			std::unordered_map<Entity, EntityInArchetype> _entityToArchetypeInfo;

			void set_up_component_common(Entity& entity, IComponent* component);
	};
}
