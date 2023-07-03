#pragma once

#include "resource_converter.h"
#include "resource_data_table.h"
#include "file_system/file.h"
#include "file_system/utils.h"
#include "utils.h"
#include "profiler/logger.h"
#include "engine_core/level/level.h"

#include <map>
#include <string>
#include <json.hpp>

namespace ad_astris::ecore
{
	struct ShaderUUIDContext;
}

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

	template<typename T>
	struct FirstCreationContext{};
	
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

				/** TODO have to fix resource name in deserialize. If object is existed, name should be taken
				 from this existed object and passed to deserialize method*/
				ResourceData resourceData;
				io::IFile* file = new io::ResourceFile(conversionContext);
				T* typedObject = new T();
				typedObject->deserialize(file);
				resourceData.file = file;
				resourceData.object = typedObject;
				resourceData.metadata.path = absolutePath;
				resourceData.metadata.type = Utils::get_enum_resource_type(typedObject->get_type());
				//resourceData.metadata.objectName = *typedObject->get_name(); Don't know if I need it
				
				write_to_disk(resourceData.file, path);
				
				_resourceDataTable->add_resource(&resourceData);
				
				return ResourceAccessor<T>(resourceData.object);
			}

			/**
			 * 
			 */
			ResourceAccessor<ecore::Level> create_level(io::URI& path);

			/**
			 * 
			 */
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

			template<typename T>
			ResourceAccessor<T> create_new_resource(FirstCreationContext<T> creationContext)
			{
				
			}

			void save_resources();
		
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

			UUID get_shader_uuid(io::URI& shaderPath, ecore::ShaderUUIDContext& shaderContext);
	};
}
