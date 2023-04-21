#pragma once

#include "engine_core/uuid.h"
#include "file_system/file_system.h"
#include "resource_converter.h"
#include "profiler/logger.h"
#include "engine_core/level.h"

#include <map>
#include <string>
#include <json.hpp>

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

		private:
			void* _resource{ nullptr };

			bool is_valid()
			{
				if (_resource == nullptr)
					return false;
				return true;
			}
	};

	// Took the idea from: https://github.com/aviktorov/scapes/blob/master/include/scapes/foundation/resources/ResourceManager.h
	struct ResourceVTable
	{
		using CreateFuncPtr = void* (*)(void* data);
		using DestroyFuncPtr = void (*)(void* resource);

		CreateFuncPtr create{ nullptr };
		DestroyFuncPtr destroy{nullptr};
	};
	
	struct Resource
	{
		void* data;
		ResourceInfo resourceInfo;
		ResourceVTable* vtable;
	};
	
	struct ResourceData
	{
		Resource* resource{ nullptr };
		io::URI pathToResource;
	};

	bool operator==(const Resource& lv, const Resource& rv);
	bool operator==(const ResourceData& lv, const ResourceData& rv);
	
	class ResourceDataTable
	{
		public:
			ResourceDataTable(io::FileSystem* fileSystem);
			~ResourceDataTable();
		
			// Load uuids and paths from aarestable file
			void load_table();

			// Upload aarestable file
			void save_table();

			bool check_resource_in_cache(UUID& uuid);

			// Need this method to understand should I reload existed resource or load new
			bool check_name_in_cache(io::URI& path);
			UUID get_uuid_by_name(io::URI& path);

			void add_resource_and_path(UUID& uuid, io::URI& path, Resource* resource = nullptr);

			// After destroying resource, its path won't be in aarestable file. 
			void destroy_resource(UUID& uuid);
		
			Resource* get_resource(UUID& uuid);
			io::URI& get_path(UUID& uuid);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			std::map<std::string, UUID> _nameToUUID;
			std::map<UUID, ResourceData> _uuidToResourceData;
			std::vector<Resource> _resources;
	};
	
	// This class should delete all resource info while shutting down engine
	class ResourceManager
	{
		public:
			ResourceManager(io::FileSystem* fileSystem);
			~ResourceManager();
		
			template<typename T>
			ResourceAccessor<T> convert_to_aares(io::URI& path)
			{
				LOG_INFO("Start converting")
				Resource* existedResource = nullptr;
				LOG_INFO("Before checking")
				if (_resourceDataTable->check_name_in_cache(path))
				{
					UUID uuid = _resourceDataTable->get_uuid_by_name(path);
					existedResource = _resourceDataTable->get_resource(uuid);
				}
				LOG_INFO("After checking")
				ResourceInfo resourceInfo;
				if (existedResource)
				{
					resourceInfo = _resourceConverter.convert_to_aares_file(path, &existedResource->resourceInfo);
				}
				else
				{
					resourceInfo = _resourceConverter.convert_to_aares_file(path);
				}
				write_to_disk(resourceInfo, path);
				ResourceVTable* vtable = _vtables[resourceInfo.type];
				Resource newResource;
				newResource.data = vtable->create(&resourceInfo);
				newResource.vtable = vtable;
				newResource.resourceInfo = resourceInfo;
				if (existedResource)
				{
					*existedResource = newResource; 
				}
				else
				{
					_resourceDataTable->add_resource_and_path(resourceInfo.uuid, path, &newResource);
				}
				
				return ResourceAccessor<T>(newResource.data);
			}
		
			ResourceAccessor<ecore::Level> load_level(io::URI& path)
			{
				// TODO
				ResourceInfo resourceInfo = read_from_disk(path);
			}
		
			template<typename T>
			ResourceAccessor<T> load_resource(io::URI& path)
			{
				// TODO rewrite ResourceInfo struct. Also, I should remake info structs for resources
				ResourceInfo resourceInfo = read_from_disk(path);
				ResourceVTable* vtable = _vtables[resourceInfo.type];
				Resource resource;
				resource.data = vtable->create(&resourceInfo);
				resource.vtable = vtable;
				resource.resourceInfo = resourceInfo;
				_resourceDataTable->add_resource_and_path(resourceInfo.uuid, path, &resource);
				
				return ResourceAccessor<T>(resource.data);
			}

			template<typename T>
			ResourceAccessor<T> get_resource(UUID uuid)
			{
				// TODO Check and async loading
				return _resourceDataTable->get_resource(uuid)->data;
			}
		
		private:
			io::FileSystem* _fileSystem;
			ResourceDataTable* _resourceDataTable;
			ResourceConverter _resourceConverter;
			std::map<ResourceType, ResourceVTable*> _vtables;

			void write_to_disk(ResourceInfo& resourceInfo, io::URI& originalPath);
			ResourceInfo read_from_disk(io::URI& path);
	};
}