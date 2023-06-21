#pragma once

#include "file_system/file_system.h"

#include <inicpp.h>

namespace ad_astris
{
	using Option = inicpp::option;
	
	class Section
	{
		friend class ConfigBase;

		using SectionIterator = inicpp::section_iterator<Option>;
		
		public:
			Section(const std::string& sectionName);
			Section(inicpp::section& section);

			bool check_option(const std::string& optionName);
			
			void set_option(Option option);
			void remove_option(const std::string& optionName);

			std::string get_name();
			uint32_t get_option_count();
		
			Option operator[](const std::string& optionName)
			{
				return _section[optionName];
			}
		
			SectionIterator begin();
			SectionIterator end();
		
		private:
			inicpp::section _section;
	};
	
	class ConfigBase
	{
		public:
			bool load_config(const io::URI& configPath);
			void unload_config();
			void save_config(io::FileSystem* fileSystem);
			//bool is_config_loaded();

			bool check_section(const std::string& sectionName);
			bool check_option(const std::string& sectionName, const std::string& optionName);

			Section get_section(const std::string& sectionName);
			inicpp::option get_option(const std::string& sectionName, const std::string& optionName);

			void set_section(Section& section);
	
		protected:
			inicpp::config _config;
			std::string _configPath;
	};

	inline void config_test(io::FileSystem* fileSystem, io::URI path)
	{
		ConfigBase config;
		LOG_INFO("Before loading config")
		config.load_config(path);
		LOG_INFO("After loading config")
		Section section = config.get_section("E:/MyEngine/MyEngine/AdAstrisEngine/bin/my_level.aalevel");
		LOG_INFO("UUID: {}", section["UUID"].get<uint64_t>())
		LOG_INFO("Type: {}", section["Type"].get<std::string>())
		LOG_INFO("Name: {}", section["Name"].get<std::string>())
		LOG_INFO("NameID: {}", section["NameID"].get<uint64_t>())
		Section section2("TestSection");
		Option option("TestOption");
		option = (uint64_t)10;
		LOG_INFO("Before set option")
		section2.set_option(option);
		LOG_INFO("After set option")
		config.set_section(section2);
		LOG_INFO("After set section")
		config.save_config(fileSystem);
		LOG_INFO("After save config")
	}

}
