#pragma once

#include "resource_pool.h"
#include "engine_core/object.h"
#include "resource_manager/resource_formats.h"
#include "file_system/file_system.h"
#include "resource_manager/utils.h"
#include "core/global_objects.h"
#include "core/config_base.h"
#include "resource_manager/resource_events.h"

namespace ad_astris::resource::impl
{
	struct ResourceDesc
	{
		io::URI path;
		ResourceType type{ ResourceType::UNDEFINED };
		ecore::ObjectName* resourceName{ nullptr };
		ecore::Object* resource{ nullptr };
	};
	
	class ResourceTable
	{
		public:
			ResourceTable(ResourcePool* resourcePool);

			void save_config();
			void save_resource(UUID uuid);
			void save_resources();
			void add_resource(const ResourceDesc& resourceDesc);

			template<typename Resource>
			Resource* load_resource(UUID uuid, ResourceType desiredResourceType)
			{
				auto it = _resourceDescByUUID.find(uuid);
				if (it == _resourceDescByUUID.end())
				{
					LOG_ERROR("ResourceTable::load_resource(): ResourceTable does not have resource with UUID {}", uuid)
					return nullptr;
				}

				ResourceDesc& resourceDesc = it->second;

				if (resourceDesc.type != desiredResourceType)
				{
					LOG_ERROR("ResourceTable::load_resource(): ResourceDesc type is {}, while passed ResourceType is {}",
						Utils::get_str_resource_type(resourceDesc.type),
						Utils::get_str_resource_type(desiredResourceType))
					return nullptr;
				}
	
				if (resourceDesc.resource)
					return static_cast<Resource*>(resourceDesc.resource);

				size_t blobSize = 0;
				uint8_t* blob = static_cast<uint8_t*>(FILE_SYSTEM()->map_to_read(resourceDesc.path, blobSize));
				io::File file(resourceDesc.path);

				if (resourceDesc.type != ResourceType::SCRIPT)
				{
					file.deserialize(blob, blobSize);
				}
				else
				{
					file.set_binary_blob(blob, blobSize);
				}

				resourceDesc.resource = _resourcePool->allocate<Resource>();
				resourceDesc.resource->deserialize(&file, resourceDesc.resourceName);
				ResourceLoadedEvent<Resource> event(static_cast<Resource*>(resourceDesc.resource));
				EVENT_MANAGER()->enqueue_event(event);

				FILE_SYSTEM()->unmap_after_reading(blob);
				return static_cast<Resource*>(resourceDesc.resource);
			}
		
			void unload_resource(UUID uuid);
			void destroy_resource(UUID uuid);

			bool is_resource_loaded(UUID uuid) const;
			bool is_resource_loaded(const std::string& name) const;
			bool is_resource_desc_valid(UUID uuid) const;
			bool is_resource_desc_valid(const std::string& name) const;
			bool is_uuid_valid(UUID uuid) const;
		
			UUID get_resource_uuid(const std::string& name) const;
			ResourceDesc* get_resource_desc(UUID uuid) const;
			ResourceType get_resource_type(UUID uuid) const;
			ecore::ObjectName* get_resource_name(UUID uuid) const;

			ResourcePool* get_resource_pool() const { return _resourcePool; }
		
		private:
			struct ResourceVTable
			{
				using DestroyFuncPtr = std::function<void(ecore::Object*)>;
				DestroyFuncPtr destroy{ nullptr };
			};
		
			Config _config;
			mutable std::mutex _mutex;
			ResourcePool* _resourcePool{ nullptr };
			std::unordered_map<std::string, UUID> _uuidByName;
			std::unordered_map<UUID, ResourceDesc> _resourceDescByUUID;
			std::unordered_map<ResourceType, ResourceVTable> _vtableByResourceType;

			void setup_resource_vtables();
			void load_config();
			void save_resource_internal(ResourceDesc* resourceDesc);
	};
}
