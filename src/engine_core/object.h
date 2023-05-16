#pragma once

#include "object_name.h"
#include "file_system/file.h"
#include "uuid.h"

#include <stdint.h>

namespace ad_astris::ecore
{
	class Object
	{
		public:
			virtual ~Object() { }
		
			virtual void serialize(io::IFile* file) { }
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) { }

			// Begin method for resources
			virtual Object* clone()
			{
				// Do I need this method?
				return nullptr;
			}

			virtual void destroy_resource() { }
		
			virtual uint64_t get_size()
			{
				return 0;
			}

			virtual bool is_resource()
			{
				return false;
			}

			virtual UUID get_uuid()
			{
				return 0;
			}

			// Returns resource type
			virtual std::string get_type()
			{
				return "NoType";
			}
			// End method for resources
		
			io::URI get_path();
			ObjectName* get_name();

			virtual std::string get_description()
			{
				return std::string{""};
			}

			bool rename(io::FileSystem* fileSystem, std::string newName);

		protected:
			ObjectName _name;
			io::URI _path;
		
			void rename_file_on_disc(io::URI& oldPath, io::URI& newPath);
			virtual void rename_in_engine(ObjectName& newName);
	};
}
