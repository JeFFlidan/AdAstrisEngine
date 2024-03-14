#pragma once

#include "fwd.h"
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
		
			virtual void serialize(io::File* file) { }
			virtual void deserialize(io::File* file, ObjectName* objectName) { }

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

			void set_path(const io::URI& path);
			io::URI get_path();
			ObjectName* get_name();

			virtual std::string get_description()
			{
				return std::string{""};
			}

			virtual void accept(resource::IResourceVisitor& resourceVisitor) { }
			bool is_dirty() const { return _isDirty; }
			void make_dirty() { _isDirty = true; }

			/** Changes filename in the engine and on disc. 
			 * @param newName can consist of two types of name. If you pass an absolute path to the file, the engine object will be
			 * renamed and the disc file will be moved to the specified location. If you pass only name, the engine object and
			 * the disc file will be renamed. You must not pass relative path
			 * @return true if renaming is successful. If you pass relative path or renaming failed, it will return false.
			 * Have to mention that the filename with extension will be considered as a relative path and absolut path
			 * must be without extension
			 */
			bool rename(const std::string& newName);

		protected:
			ObjectName* _name{ nullptr };
			io::URI _path;
			bool _isDirty{ false };
	};
}
