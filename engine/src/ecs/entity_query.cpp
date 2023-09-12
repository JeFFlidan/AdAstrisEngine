#include "entity_query.h"
#include "execution_context.h"

using namespace ad_astris::ecs;

void EntityQuery::for_each_chunk(std::function<void(ExecutionContext&)> executeFunction)
{
	for (auto i = 0; i != _archetypes.size(); ++i)
	{
		Archetype* archetype = _archetypes[i];
		ExecutionContext& executionContext = _executionContexts[i];
		
		for (auto j = 0; j != archetype->get_chunks_count(); ++j)
		{
			executionContext.set_chunk_index(j);
			executeFunction(executionContext);
		}

		executionContext.clear_loaded_subchunks();
	}
}
