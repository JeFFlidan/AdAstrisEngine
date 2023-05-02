#pragma once

#include "engine_core/uuid.h"
#include "engine_core/object.h"
#include "file_system/file_system.h"
#include "file_system/file.h"
#include "file_system/utils.h"
#include "resource_converter.h"
#include "resource_formats.h"
#include "utils.h"
#include "profiler/logger.h"
#include "engine_core/level/level.h"

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

	// Metadata from resource table. When resource table is saved, this metadata is used to write
	// info about the resource if it wasn't loaded into memory
	struct ResourceMetadata
	{
		io::URI path;
		ResourceType type{ ResourceType::UNDEFINED };
		ecore::ObjectName* objectName{ nullptr };
	};

	struct ResourceData
	{
		io::IFile* file{ nullptr };
		ecore::Object* object{ nullptr };
		ResourceMetadata metadata;
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

			// Checks if a resource was loaded
			bool check_resource_in_table(UUID& uuid);

			// Need this method to understand should I reload existed resource or load new
			bool check_name_in_table(io::URI& path);
			bool check_name_in_table(ecore::ObjectName& name);
		
			// Checks if a UUID is in a table. If not, the the resource doesn't exist and UUID is wrong 
			bool check_uuid_in_table(UUID& uuid);

			void add_resource(ResourceData* resource);

			// After destroying resource, its path won't be in aarestable file. 
			void destroy_resource(UUID& uuid);
		
			UUID get_uuid_by_name(io::URI& path);
			UUID get_uuid_by_name(ecore::ObjectName& name);
			io::IFile* get_resource_file(UUID& uuid);
			ecore::Object* get_resource_object(UUID& uuid);
			ResourceData* get_resource_data(UUID& uuid);
			io::URI get_path(UUID& uuid);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			std::map<std::string, UUID> _nameToUUID;
			std::map<UUID, ResourceData> _uuidToResourceData;
	};
	
	// ResourceManager is responsible for loading levels and managing resources 
	// (destroy, save, update, etc.) 
	// Also, ResourceManager is used to convert files from DCC tools to a custom '.aares' file.
	class ResourceManager
	{
		public:
			ResourceManager(io::FileSystem* fileSystem);
			~ResourceManager();

			/** Convert file from DCC tools to a custom '.aares' file. \n
			 * Supported file formats for 3D-models: gltf, obj
			 * Supported file formats for textures: tga, png, jpg, tiff
			 * @param path should be a valid path. Otherwise, nullptr will be returned
			 */
			template <typename T>
			ResourceAccessor<T> convert_to_aares(io::URI& path)
			{
				if (!io::Utils::exists(_fileSystem, path))
				{
					LOG_ERROR("ResourceManager::convert_to_aares(): Invalid path {}", path.c_str())
					return nullptr;
				}

				ecore::Object* existedObject = nullptr;
				if (_resourceDataTable->check_name_in_table(path))
				{
					UUID uuid = _resourceDataTable->get_uuid_by_name(path);

					// Have to think is it a good idea to delete existing object and file before reloading
					existedObject = _resourceDataTable->get_resource_object(uuid);
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
				
				ResourceData resourceData;
				io::IFile* file = new io::ResourceFile(conversionContext);
				T* typedObject = new T();
				typedObject->deserialize(file);
				resourceData.file = file;
				resourceData.object = typedObject;
				resourceData.metadata.path = absolutePath;
				resourceData.metadata.type = Utils::get_enum_resource_type(typedObject->get_type());
				
				write_to_disk(resourceData.file, path);
				
				_resourceDataTable->add_resource(&resourceData);
				
				return ResourceAccessor<T>(resourceData.object);
			}
		
			ResourceAccessor<ecore::Level> load_level(io::URI& path);
		
			/** If the resource has been loaded, the method returns the resource data. Otherwise, the resource will be loaded from disc
			 * @param uuid should be valid uuid. 
			 */
			template<typename T>
			ResourceAccessor<T> get_resource(UUID uuid)
			{
				if (!_resourceDataTable->check_uuid_in_table(uuid))
				{
					LOG_ERROR("ResourceManager:get_resource(): Invalid UUID {}", uuid)
					return nullptr;
				}
				if (_resourceDataTable->check_resource_in_table(uuid))
				{
					LOG_INFO("Get resource object")
					return _resourceDataTable->get_resource_object(uuid);
				}
				
				return load_resource<T>(uuid);
			}

			void save_resources();
		
			ResourceType get_resource_type(ecore::ObjectName& objectName);
			ResourceType get_resource_type(UUID uuid);
		
		private:
			io::FileSystem* _fileSystem;
			ResourceDataTable* _resourceDataTable;
			ResourceConverter _resourceConverter;

			void write_to_disk(io::IFile* file, io::URI& originalPath);
			io::IFile* read_from_disk(io::URI& path);
		
			template<typename T>
			ResourceAccessor<T> load_resource(UUID& uuid)
			{
				io::URI path = _resourceDataTable->get_path(uuid);
				io::IFile* file = read_from_disk(path);

				ResourceData* resource = _resourceDataTable->get_resource_data(uuid);
				T* typedObject = new T();
				typedObject->deserialize(file, resource->metadata.objectName);
				resource->file = file;
				resource->object = typedObject;
					
				return ResourceAccessor<T>(resource->object);
			}
	};
}
