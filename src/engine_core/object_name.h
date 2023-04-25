#pragma once

#include <stdint.h>
#include <map>
#include <string>

namespace ad_astris::ecore
{
	class NameID
	{
		public:
			uint32_t get_id();
			void operator++();
			void operator--();
			NameID& operator=(const NameID& other);
			operator uint32_t();
		
		private:
			uint32_t _id{ 0 };
	};

	/** Name for engine objects. The name consists of a string + the number of instances of the name. \n 
	 * \n If count = 0, no number will be added to the name.
	 * Otherwise, _{instances count} will be appended to the name \n
	 * \n 128 is maximum name size 
	 */
	class ObjectName
	{
		public:
			ObjectName(std::string newName);
			~ObjectName();

			void change_name(std::string newName);
			void destroy_name();
			std::string get_name();

			static void tests();
		
			ObjectName& operator=(const ObjectName& other);
			bool operator==(const ObjectName& name) const;
			bool operator<(const ObjectName& name) const;
		
		private:
			char* _name;
			NameID _nameID;
			static std::map<ObjectName, NameID> _nameTable;

			void delete_name_from_table();
	};
}