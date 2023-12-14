#pragma once

#include "core/object_pool.h"
#include <set>

namespace ad_astris::vulkan
{
	class VulkanDevice;
	class IVulkanObject;
	
	class VulkanObjectPool : private ObjectPool
	{
		public:
			VulkanObjectPool();

			template<typename ResourceType, typename... ARGS>
			ResourceType* allocate(ARGS&&... args)
			{
				ResourceType* object = ObjectPool::allocate<ResourceType>(std::forward<ARGS>(args)...);
				_vkObjects.insert(reinterpret_cast<IVulkanObject*>(object));
				return object;
			}

			template<typename ResourceType>
			void free(ResourceType* resource)
			{
				auto it = _vkObjects.find(resource);
				if (it != _vkObjects.end())
					_vkObjects.erase(it);
				ObjectPool::free(resource);
			}

			void cleanup(VulkanDevice* device);

		private:
			std::set<IVulkanObject*> _vkObjects;
	};
}
