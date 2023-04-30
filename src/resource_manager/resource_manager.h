#pragma once

#include "engine_core/uuid.h"
#include "engine_core/object.h"
#include "file_system/file_system.h"
#include "file_system/file.h"
#include "file_system/utils.h"
#include "resource_converter.h"
#include "resource_formats.h"
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

	struct ResourceData
	{
		io::IFile* file{ nullptr };
		ecore::Object* object{ nullptr };
	};
	
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

			void add_resource(ResourceData* resource);

			// After destroying resource, its path won't be in aarestable file. 
			void destroy_resource(UUID& uuid);
		
			io::IFile* get_resource_file(UUID& uuid);
			ecore::Object* get_resource_object(UUID& uuid);
			io::URI get_path(UUID& uuid);
			ResourceType get_resource_type(UUID& uuid);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			std::map<std::string, UUID> _nameToUUID;
			std::map<UUID, ResourceData> _uuidToResourceData;
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
				io::IFile* existedFile = nullptr;
				ecore::Object* existedObject = nullptr;
				if (_resourceDataTable->check_name_in_cache(path))
				{
					UUID uuid = _resourceDataTable->get_uuid_by_name(path);

					// Have to think is it a good idea to delete existing object and file before reloading
					existedObject = _resourceDataTable->get_resource_object(uuid);
					existedFile = _resourceDataTable->get_resource_file(uuid);
				}

				io::ConversionContext<T> conversionContext;
				if (existedObject)
				{
					_resourceConverter.convert_to_aares_file(path, &conversionContext, existedObject);
				}
				else
				{
					_resourceConverter.convert_to_aares_file(path, &conversionContext);
				}
				
				io::URI relPath = std::string("assets\\" + io::Utils::get_file_name(path) + ".aares").c_str();
				io::URI absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem, relPath);
				conversionContext.filePath = absolutePath.c_str();
				io::URI path1 = conversionContext.filePath.c_str();
				
				ResourceData resourceData;
				io::IFile* file = new io::ResourceFile(conversionContext);
				T* typedObject = new T();
				typedObject->deserialize(file);
				resourceData.file = file;
				resourceData.object = typedObject;
				
				write_to_disk(resourceData.file, path);
				
				_resourceDataTable->add_resource(&resourceData);

				if (existedObject && existedFile)
				{
					delete existedObject;
					delete existedFile;
				}
				
				return ResourceAccessor<T>(resourceData.object);
			}
		
			ResourceAccessor<ecore::Level> load_level(io::URI& path)
			{
				// TODO
				//ResourceInfo resourceInfo = read_from_disk(path);
			}
		
			template<typename T>
			ResourceAccessor<T> load_resource(UUID& uuid)
			{
				// TODO rewrite ResourceInfo struct. Also, I should remake info structs for resources
				io::URI path = _resourceDataTable->get_path(uuid);
				io::IFile* file = read_from_disk(path);
				ResourceData resource;
				T* typedObject = new T();
				typedObject->deserialize(file);
				resource.file = file;
				resource.object = typedObject;
				_resourceDataTable->add_resource(&resource);
				
				return ResourceAccessor<T>(resource.object);
			}
		
			/** If the resource has been loaded, the method returns the resource data. Otherwise, the resource will be loaded from disc
			 * @param uuid should be valid uuid. 
			 */
			template<typename T>
			ResourceAccessor<T> get_resource(UUID uuid)
			{
				// TODO Check and async loading
				return _resourceDataTable->get_resource_object(uuid);
			}

			void save_resources()
			{
				// TODO
			}
		
		private:
			io::FileSystem* _fileSystem;
			ResourceDataTable* _resourceDataTable;
			ResourceConverter _resourceConverter;

			void write_to_disk(io::IFile* file, io::URI& originalPath);
			io::IFile* read_from_disk(io::URI& path);
	};
}