#pragma once

#include "non_copyable_non_movable.h"
#include "pool_allocator.h"
#include "reflection.h"

namespace ad_astris
{
	class ObjectPool : public NonCopyableNonMovable
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
			void create_pool_for_new_resource()
			{
				std::string typeName = get_type_name<ResourceType>();
				auto it = _poolAllocatorByTypeName.find(typeName);
				if (it != _poolAllocatorByTypeName.end())
				{
					return;
				}
				auto typedAllocator = std::make_unique<TypedPoolAllocator<ResourceType>>();
				_poolAllocatorByTypeName[typeName] = std::move(typedAllocator);
			}

			void cleanup()
			{
				for (auto& alloc : _poolAllocatorByTypeName)
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

			std::unordered_map<std::string, std::unique_ptr<GeneralPoolAllocator>> _poolAllocatorByTypeName;

			template<typename ResourceType>
			GeneralPoolAllocator* common_check()
			{
				std::string typeName = get_type_name<ResourceType>();
				auto it = _poolAllocatorByTypeName.find(typeName);
				if (it == _poolAllocatorByTypeName.end())
				{
					LOG_FATAL("ObjectPool::alocate: Can't allocate resource because there is no pool allocator for type {}", typeName)
				}
				return it->second.get();
			}
	};
}
