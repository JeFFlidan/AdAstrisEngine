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
			NameID() = default;
			NameID(uint32_t newId);
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
			NameIDTable(NameID nameId);

			NameID get_next_id();
			void add_id(NameID nameId);
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
			ObjectName(const char* newName, NameID nameId);
			~ObjectName();
		
			void change_name(const char* newName);
			void change_name(ObjectName& otherName);
			// Destroy the char array of the name. Also, remove the name from the
			// name table or decrease instance count of the name
			void destroy_name();
			std::string get_full_name();
			std::string get_name_without_id();
			NameID get_name_id();

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
			void add_new_name_to_table(NameID nameID = 0);
	};
}