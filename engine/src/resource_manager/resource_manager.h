#pragma once

#include "resource_converter.h"
#include "resource_data_table.h"
#include "resource_pool.h"
#include "resource_events.h"
#include "utils.h"
#include "file_system/file.h"
#include "file_system/utils.h"
#include "events/event_manager.h"
#include "profiler/logger.h"
#include "engine_core/level/level.h"

#include <string>
#include <json.hpp>

namespace ad_astris::ecore
{
	class Shader;
	namespace material
	{
		struct ShaderUUIDContext;
		struct ShaderHandleContext;
	}
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

	struct BuiltinResourcesContext
	{
		std::vector<UUID> materialTemplateNames;

		void clear();
	};
	
	// ResourceManager is responsible for loading levels and managing resources 
	// (destroy, save, update, etc.) 
	// Also, ResourceManager is used to convert files from DCC tools to a custom '.aares' file.
	class ResourceManager
	{
		public:
			ResourceManager(io::FileSystem* fileSystem, events::EventManager* eventManager);
			~ResourceManager();

			io::FileSystem* get_file_system()
			{
				return _fileSystem;
			}

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

				if (io::Utils::is_relative(path))
				{
					path = io::Utils::get_absolute_path_to_file(_fileSystem, path);
				}

				ecore::Object* existedObject = nullptr;
				ecore::ObjectName* existedObjectName = nullptr;
				if (_resourceDataTable.check_name_in_table(path))
				{
					UUID uuid = _resourceDataTable.get_uuid_by_name(path);

					// Have to think is it a good idea to delete existing object and file before reloading
					existedObject = _resourceDataTable.get_resource_object(uuid);
					existedObjectName = _resourceDataTable.get_resource_data(uuid)->metadata.objectName;
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
				
				io::URI relPath = std::string("content\\" + io::Utils::get_file_name(path) + ".aares").c_str();
				io::URI absolutePath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), relPath);
				conversionContext.filePath = absolutePath.c_str();

				/** TODO have to fix resource name in deserialize. If object is existed, name should be taken
				 from this existed object and passed to deserialize method*/
				ResourceData resourceData{};
				resourceData.metadata.path = absolutePath;
				resourceData.metadata.objectName = existedObjectName ? existedObjectName : _resourcePool.allocate<ecore::ObjectName>(io::Utils::get_file_name(absolutePath).c_str());
				
				io::IFile* file = _resourcePool.allocate<io::ResourceFile>(conversionContext);
				T* typedObject = _resourcePool.allocate<T>();
				typedObject->deserialize(file, resourceData.metadata.objectName);
				
				resourceData.file = file;
				resourceData.object = typedObject;
				resourceData.metadata.type = Utils::get_enum_resource_type(typedObject->get_type());
				
				send_resource_event(typedObject);			// I have to make it in another thread.
				write_to_disk(resourceData.file, path);
				
				_resourceDataTable.add_resource(&resourceData);
				
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

			template<typename T>
			ResourceAccessor<T> get_resource(const std::string& resourceName)
			{
				if (!_resourceDataTable.check_name_in_table(resourceName))
				{
					LOG_ERROR("ResourceManager:get_resource(): Invalid name {}", resourceName)
					return nullptr;
				}
					
				return get_resource<T>(_resourceDataTable.get_uuid_by_name(resourceName));
			}
		
			/** If the resource has been loaded, the method returns the resource data. Otherwise, the resource will be loaded from disc
			 * @param uuid should be valid uuid. 
			 */
			template<typename T>
			ResourceAccessor<T> get_resource(UUID uuid)
			{
				if (!_resourceDataTable.check_uuid_in_table(uuid))
				{
					LOG_ERROR("ResourceManager:get_resource(): Invalid UUID {}", uuid)
					return nullptr;
				}
				if (_resourceDataTable.check_resource_in_table(uuid))
				{
					LOG_INFO("Get resource object")
					return _resourceDataTable.get_resource_object(uuid);
				}

				return load_resource<T>(uuid);
			}

			template<typename T>
			ResourceAccessor<T> create_new_resource(FirstCreationContext<T>& creationContext);

			void save_resources();
			void destroy_resource(UUID uuid);
		
		private:
			io::FileSystem* _fileSystem;
			events::EventManager* _eventManager;
			ResourceDataTable _resourceDataTable;
			ResourceConverter _resourceConverter;
			ResourcePool _resourcePool;

			BuiltinResourcesContext _builtinResourcesContext;

			void write_to_disk(io::IFile* file, io::URI& originalPath);
			io::IFile* read_from_disk(io::URI& path, bool isShader = false);
		
			template<typename T>
			ResourceAccessor<T> load_resource(UUID& uuid)
			{
				io::URI path = _resourceDataTable.get_resource_path(uuid);
				bool isShader = _resourceDataTable.get_resource_type(uuid) == ResourceType::SHADER;

				io::IFile* file = read_from_disk(path, isShader);

				if (isShader)
				{
					std::string metadata = std::to_string(uuid);
					file->set_metadata(metadata);
				}

				ResourceData* resource = _resourceDataTable.get_resource_data(uuid);
				T* typedObject = _resourcePool.allocate<T>();
				typedObject->deserialize(file, resource->metadata.objectName);
				resource->file = file;
				resource->object = typedObject;
		
				send_resource_event(typedObject);
				
				return ResourceAccessor<T>(resource->object);
			}

			template<typename T>
			void send_resource_event(T* resourceObject);
		
			void load_builtin_resources();
			void add_shader_to_uuid_context(io::URI& shaderPath, ecore::material::ShaderUUIDContext& shaderUUIDContext);
			void load_shader(UUID& shaderUUID, ecore::material::ShaderHandleContext& shaderContext);
	};
}
