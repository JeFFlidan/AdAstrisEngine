#pragma once

#include "api.h"
#include "entity_types.h"
#include <cstdint>
#include <algorithm>
#include <functional>

namespace ad_astris::ecs
{
	class EntitySystem;
	class Archetype;

	class ECS_API Subchunk
	{
		public:
			Subchunk() = default;
			Subchunk(uint8_t* startPtr, uint32_t subchunkSize, uint16_t structureSize)
				: _startPtr(startPtr), _subchunkSize(subchunkSize), _structureSize(structureSize) { }

			uint8_t* get_ptr() const
			{
				return _startPtr;
			}

			uint32_t get_subchunk_size() const
			{
				return _subchunkSize;
			}

			uint16_t get_structure_size() const
			{
				return _structureSize;
			}
		
		private:
			uint8_t* _startPtr{ nullptr };
			uint32_t _subchunkSize{ 0 };
			uint16_t _structureSize{ 0 };
	};
	
	class ECS_API ArchetypeHandle
	{
		ECS_API friend class EntitySystem;
		
		public:
			ArchetypeHandle() = default;
			uint32_t get_id()
			{
				return _id;
			}

		private:
			ArchetypeHandle(uint32_t id) : _id(id) {}
			uint32_t _id;
	};

	// Can be passed into EntitySystem only once because all data will be moved 
	class ECS_API ArchetypeCreationContext
	{
		ECS_API friend Archetype;
		ECS_API friend EntitySystem;
		
		public:
			template<typename ...TYPES>
			void add_components_id()
			{
				((set_up(Component<TYPES>::_typeId, sizeof(TYPES))), ...);
				std::sort(_ids.begin(), _ids.end());
			}

			void add_components_id(std::vector<uint32_t>& ids)
			{
				// TODO Should improve
				_ids = std::move(ids);
			}

		protected:
			std::vector<uint32_t> _ids;
			std::unordered_map<uint32_t, uint16_t> _idToSize;
			uint32_t _allComponentsSize{ 0 };

			void set_up(uint32_t id, uint64_t size)
			{
				_allComponentsSize += size;
				_idToSize[id] = size;
				_ids.push_back(id);
			}

	};

	class ECS_API ArchetypeExtensionContext : public ArchetypeCreationContext
	{
		friend EntitySystem;
		
		public:
			ArchetypeExtensionContext(ArchetypeHandle srcArchetype) : _srcArchetype(srcArchetype)
			{
				
			}

		private:
			ArchetypeHandle _srcArchetype;
	};
}
