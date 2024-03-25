#pragma once

#include "pool_allocator.h"
#include "reflection.h"
#include <typeindex>

namespace ad_astris
{
	class ObjectPool
	{
		public:
			template<typename ResourceType, typename... ARGS>
			ResourceType* allocate(ARGS&&... args)
			{
				GeneralPoolAllocator* poolAllocator = common_check<ResourceType>();
				ResourceType* resource = static_cast<ResourceType*>(poolAllocator->allocate());
				new(resource) ResourceType(std::forward<ARGS>(args)...);
				return resource;
			}

			template<typename ResourceType>
			void free(ResourceType* resource)
			{
				GeneralPoolAllocator* poolAllocator = common_check<ResourceType>();
				poolAllocator->free(resource);
			}

			template<typename ResourceType>
			void create_pool_for_new_resource(uint32_t initialObjectCount = 64)
			{
				auto it = _poolAllocatorByTypeIndex.find(std::type_index(typeid(ResourceType)));
				if (it != _poolAllocatorByTypeIndex.end())
				{
					return;
				}
				std::unique_ptr<GeneralPoolAllocator> allocator = std::make_unique<TypedPoolAllocator<ResourceType>>();
				allocator->allocate_new_pool(initialObjectCount);
				_poolAllocatorByTypeIndex[std::type_index(typeid(ResourceType))] = std::move(allocator);
			}

			void cleanup()
			{
				for (auto& alloc : _poolAllocatorByTypeIndex)
					alloc.second->cleanup();
			}

		private:
			class GeneralPoolAllocator
			{
				public:
					virtual ~GeneralPoolAllocator() { } 
					void* allocate()
					{
						return allocate_internal();
					}

					void allocate_new_pool(uint32_t objectCount)
					{
						allocate_new_pool_internal(objectCount);
					}

					template<typename ResourceType>
					void free(ResourceType* resource)
					{
						free_internal(resource);
					}

					void cleanup()
					{
						cleanup_internal();
					}

				protected:
					virtual void* allocate_internal() { return nullptr; }
					virtual void allocate_new_pool_internal(uint32_t objectCount) { }
					virtual void free_internal(void* resource) { }
					virtual void cleanup_internal() { }
			};

			template<typename ResourceType>
			class TypedPoolAllocator : public GeneralPoolAllocator
			{
				private:
					ThreadSafePoolAllocator<ResourceType> _pool;

					virtual void* allocate_internal() override
					{
						return _pool.allocate();
					}

					virtual void allocate_new_pool_internal(uint32_t objectCount) override
					{
						_pool.allocate_new_pool(objectCount);
					}

					virtual void free_internal(void* resource) override
					{
						ResourceType* typedResource = static_cast<ResourceType*>(resource);
						_pool.free(typedResource);
					}

					virtual void cleanup_internal() override
					{
						_pool.cleanup();
					}
			};

			std::unordered_map<std::type_index, std::unique_ptr<GeneralPoolAllocator>> _poolAllocatorByTypeIndex;

			template<typename ResourceType>
			GeneralPoolAllocator* common_check()
			{
				auto it = _poolAllocatorByTypeIndex.find(std::type_index(typeid(ResourceType)));
				if (it == _poolAllocatorByTypeIndex.end())
				{
					LOG_FATAL("ObjectPool::alocate: Can't allocate resource because there is no pool allocator for type {}", get_type_name<ResourceType>())
				}
				return it->second.get();
			}
	};
}
