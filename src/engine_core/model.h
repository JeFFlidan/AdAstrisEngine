#pragma once

#include "resource_manager/resource_formats.h"

namespace ad_astris
{
	namespace ecore
	{
		struct Model
		{
			resource::ModelInfo* info;

			uint8_t* vertices;
			uint64_t verticesCount;

			uint8_t* indices;
			uint64_t indicesCount;
		};
	}

	namespace resource
	{
		template<>
		struct ResourceMethods<ecore::Model>
		{
			static void* create(void* data);
			static void destroy(void* resource);
		};
	}
}