#pragma once

#include <stdint.h>
#include <map>
#include <vector>
#include <string>

namespace ad_astris::ecore
{
	void tests();
	
	class NameID
	{
		public:
			uint32_t get_id();
			void operator++();
			void operator--();
			NameID& operator=(const NameID& other);
			bool operator==(const NameID& other);
			operator uint32_t();
		
		private:
			uint32_t _id{ 0 };
	};

	class NameIDTable
	{
		public:
			NameIDTable();

			NameID get_next_id();
			void remove_id(NameID nameId);
			bool is_empty();
		
		private:
			std::vector<NameID> _removedNameIDs;
			std::vector<NameID> _table;
	};


	/** Name for engine objects. The name consists of a string + the number of instances of the name. \n 
	 * \n If count = 0, no number will be added to the name.
	 * Otherwise, _{instances count} will be appended to the name \n
	 * \n 128 is maximum name size 
	 */
	class ObjectName
	{
		public:
			ObjectName() = default;
			ObjectName(const char* newName);
			~ObjectName();
		
			void change_name(const char* newName);
			void change_name(ObjectName& otherName);
			// Destroy the char array of the name. Also, remove the name from the
			// name table or decrease instance count of the name
			void destroy_name();
			std::string get_string();
			std::string get_name_without_instance();

			// Destroy name table
			static void cleanup();

			ObjectName& operator=(const ObjectName& otherName);
			bool operator==(const ObjectName& name) const;
			bool operator<(const ObjectName& name) const;
		
		private:
			char* _name{ nullptr };
			NameID _nameID;
		
			static std::map<ObjectName, NameIDTable> _nameTable;
		
			void delete_name_from_table();
			void add_new_name_to_table();
	};
}