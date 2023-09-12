#pragma once

#include "file_system/file_system.h"
#include "profiler/logger.h"

#include <inicpp/inicpp.h>

namespace ad_astris
{
	using Option = inicpp::option;
	using InternalSectionIterator = inicpp::section_iterator<Option>;
	using InternalConfigIterator = inicpp::config_iterator<inicpp::section>;
	
	class Section
	{
		friend class Config;
		
		public:
			Section(const std::string& sectionName);
			Section(inicpp::section& section);

			bool check_option(const std::string& optionName);
			
			void set_option(Option option);
		
			template<typename T>
			void set_option(const std::string& optionName, T optionValue)
			{
				Option option(optionName);
				option = optionValue;
				set_option(option);
			}
		
			void remove_option(const std::string& optionName);

			std::string get_name();
			uint32_t get_option_count();

			template<typename T>
			T get_option_value(const std::string& optionName)
			{
				return _section[optionName].get<T>();
			}
		
			Option operator[](const std::string& optionName)
			{
				return _section[optionName];
			}
		
			class SectionIterator
			{
				public:
					SectionIterator(InternalSectionIterator internalIter) : _internalIter(internalIter)
					{
						
					}

					SectionIterator& operator++()
					{
						++_internalIter;
						return *this;
					}

					Option operator* ()
					{
						return Option(*_internalIter);
					}

					bool operator==(const SectionIterator& other)
					{
						return _internalIter == other._internalIter;
					}

					bool operator!=(const SectionIterator other)
					{
						return _internalIter != other._internalIter; 
					}
				
				private:
					InternalSectionIterator _internalIter;
			};
		
			SectionIterator begin();
			SectionIterator end();
		
		private:
			inicpp::section _section;
	};

	
	class Config
	{
		public:
			bool load_from_file(const io::URI& configPath);
			void unload();
			void save(io::FileSystem* fileSystem);

			bool check_section(const std::string& sectionName);
			bool check_option(const std::string& sectionName, const std::string& optionName);

			Section get_section(const std::string& sectionName);
			inicpp::option get_option(const std::string& sectionName, const std::string& optionName);

			void set_section(Section& section);
		
			template<typename T>
			void set_option(const std::string& sectionName, const std::string& optionName, T& value)
			{
				if (!check_section(sectionName))
				{
					LOG_ERROR("ConfigBase::set_option(): There is no section {} in config file {}", sectionName, _configPath)
					return;
				}

				_config.add_option(sectionName, optionName, value);
			}

			class ConfigIterator
			{
				public:
					ConfigIterator(InternalConfigIterator internalIter) : _internalIter(internalIter)
					{
							
					}

					ConfigIterator& operator++()
					{
						++_internalIter;
						return *this;
					}

					Section operator* ()
					{
						return Section(*_internalIter);
					}

					bool operator==(const ConfigIterator& other)
					{
						return _internalIter == other._internalIter;
					}

					bool operator!=(const ConfigIterator other)
					{
						return _internalIter != other._internalIter; 
					}
				
				private:
					InternalConfigIterator _internalIter;
			};
		
			ConfigIterator begin();
			ConfigIterator end();
		
		protected:
			std::string _configPath;

		private:
			inicpp::config _config;
	};

	inline void config_test(io::FileSystem* fileSystem, io::URI path)
	{
		Config config;
		LOG_INFO("Before loading config")
		config.load_from_file(path);
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
		config.save(fileSystem);
		LOG_INFO("After save config")
	}

}
