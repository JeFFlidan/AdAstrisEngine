#pragma once

#include "file_system/file_system.h"
#include "engine_core/object.h"
#include "resource_formats.h"

#include <unordered_map>

namespace ad_astris::resource
{
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

			// Upload .aares and .aalevel files
			void save_resources();

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
}