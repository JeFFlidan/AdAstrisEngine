#include "execution_context.h"

using namespace ad_astris::ecs;

ExecutionContext::ExecutionContext(Archetype* archetype, std::unordered_map<uint32_t, ComponentAccess>& accessByComponentID)
	: _accessByComponentID(accessByComponentID), _archetype(archetype)
{

}

void ExecutionContext::set_chunk_index(uint32_t chunkIndex)
{
	_chunkIndex = chunkIndex;
}

uint32_t ExecutionContext::get_entities_count()
{
	return _archetype->get_entities_count_per_chunk(_chunkIndex);
}

void ExecutionContext::clear_loaded_subchunks()
{
	_loadedSubchunks.clear();
}
