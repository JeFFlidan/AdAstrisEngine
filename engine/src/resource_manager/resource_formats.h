#pragma once

#include "profiler/logger.h"
#include "engine_core/uuid.h"
#include <vector>
#include <string>
#include <functional>

namespace ad_astris::resource
{
	template<typename T>
	class ResourceAccessor
	{
		public:
			ResourceAccessor(void* resource) : _resource(resource) {}

			T* get_resource()
			{
				if (is_valid())
				{
					return static_cast<T*>(_resource);
				}
				LOG_ERROR("ResourceAccessor::get_resource(): Invalid pointer to the resource")
				return nullptr;
			}

			bool is_valid()
			{
				if (_resource == nullptr)
					return false;
				return true;
			}
		
		private:
			void* _resource{ nullptr };
	};

	
	enum class ResourceType
	{
		UNDEFINED,
		MODEL,
		TEXTURE,
		LEVEL,
		MATERIAL,
		MATERIAL_TEMPLATE,
		SHADER
	};
	
	struct LevelEngineInfo
	{
		UUID uuid;
		std::vector<UUID> texturesUUID;
	};

	struct LevelFileInfo
	{
		UUID uuid;
		std::vector<std::string> modelInstancesInfo;
		std::vector<std::string> texturesInfo;
	};
}
