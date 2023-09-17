#pragma once

#include "resource_pool.h"
#include "resource_visitor.h"
#include "file_system/file_system.h"
#include "engine_core/object.h"
#include "resource_formats.h"
#include "core/config_base.h"
#include <mutex>

namespace ad_astris::resource
{
	// Metadata from resource table. When resource table is saved, this metadata is used to write
	// info about the resource if it wasn't loaded into memory
	struct ResourceMetadata
	{
		io::URI path;
		ResourceType type{ ResourceType::UNDEFINED };
		ecore::ObjectName* objectName{ nullptr };
		bool builtin{ false };
	};

	struct ResourceData
	{
		io::File* file{ nullptr };
		ecore::Object* object{ nullptr };
		ResourceMetadata metadata;
	};

	struct BuiltinResourcesContext;
	
	class ResourceDataTable
	{
		public:
			ResourceDataTable() = default;
			ResourceDataTable(io::FileSystem* fileSystem, ResourcePool* resourcePool);
			~ResourceDataTable();

			ResourceDataTable& operator=(const ResourceDataTable& other)
			{
				if (this == &other)
					return *this;
				_fileSystem = other._fileSystem;
				_resourcePool = other._resourcePool;
				_resourceDeleterVisitor = other._resourceDeleterVisitor;
				_uuidToResourceData = other._uuidToResourceData;
				_nameToUUID = other._nameToUUID;
				_config = other._config;
				return *this;
			}
			
			// Load uuids and paths from aarestable file
			void load_table(BuiltinResourcesContext& context);

			// Upload aarestable file
			void save_table();

			// Upload .aares and .aalevel files
			void save_resources();

			// Checks if a resource was loaded
			bool check_resource_in_table(UUID& uuid);

			// Need this method to understand should I reload existed resource or load new
			bool check_name_in_table(io::URI& path);
			bool check_name_in_table(const std::string& name);
			
			// Checks if a UUID is in a table. If not, the the resource doesn't exist and UUID is wrong 
			bool check_uuid_in_table(UUID& uuid);

			void add_resource(ResourceData* resource);
			void add_empty_resource(ResourceData* resource, UUID uuid);

			// After destroying resource, its path won't be in aarestable file. 
			void destroy_resource(UUID& uuid);
			
			UUID get_uuid_by_name(io::URI& path);
			UUID get_uuid_by_name(const std::string& name);
		
			io::File* get_resource_file(UUID& uuid);
			ecore::Object* get_resource_object(UUID& uuid);
			ResourceData* get_resource_data(UUID& uuid);
			ResourceType get_resource_type(UUID& uuid);
			io::URI get_resource_path(UUID& uuid);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			ResourcePool* _resourcePool{ nullptr };
			ResourceDeleterVisitor _resourceDeleterVisitor;
			std::map<std::string, UUID> _nameToUUID;
			std::map<UUID, ResourceData> _uuidToResourceData;
			Config _config;
			std::mutex _mutex;
	};
}