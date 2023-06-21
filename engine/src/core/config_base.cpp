#include "config_base.h"
#include "file_system/utils.h"

using namespace ad_astris;

Section::Section(const std::string& sectionName) : _section(inicpp::section(sectionName))
{

}

Section::Section(inicpp::section& section) : _section(section)
{
	
}

bool Section::check_option(const std::string& optionName)
{
	return _section.contains(optionName);
}

void Section::set_option(Option option)
{
	if (check_option(option.get_name()))
	{
		LOG_WARNING("Section::set_option(): Section {} has option {}. This option will be overwritter.", _section.get_name(), option.get_name())
		remove_option(option.get_name());
	}

	_section.add_option(option);
}

void Section::remove_option(const std::string& optionName)
{
	if (check_option(optionName))
		_section.remove_option(optionName);

	LOG_WARNING("Section::remove_option(): Section {} doesn't contain option with name {}", _section.get_name(), optionName)
}

std::string Section::get_name()
{
	return _section.get_name();
}

uint32_t Section::get_option_count()
{
	return _section.size();
}

Section::SectionIterator Section::begin()
{
	return SectionIterator(_section.end());
}

Section::SectionIterator Section::end()
{
	return SectionIterator(_section.begin());
}

bool Config::load_from_file(const io::URI& configPath)
{
	_configPath = configPath.c_str();
	if (io::Utils::get_file_extension(configPath) != "ini")
	{
		LOG_ERROR("ConfigBase::load_config(): You can't load config if it's not an .ini file")
		return false;
	}
	if (!io::Utils::is_absolute(configPath))
	{
		LOG_ERROR("ConfigBase::load_config(): You passed not absolute path. You can get absolute path using FileSystem")
		return false;
	}
	_config = inicpp::parser::load_file(configPath.c_str());
	return true;
}

void Config::unload()
{
	_config.clear_config();
}

void Config::save(io::FileSystem* fileSystem)
{
	io::Stream* stream = fileSystem->open(_configPath.c_str(), "w");
	fileSystem->close(stream);
	inicpp::parser::save(_config, _configPath);
}

bool Config::check_section(const std::string& sectionName)
{
	return _config.contains(sectionName);		// ??
}

bool Config::check_option(const std::string& sectionName, const std::string& optionName)
{
	if (check_section(sectionName))
	{
		return _config[sectionName].contains(optionName);
	}
}

Section Config::get_section(const std::string& sectionName)
{
	if (!check_section(sectionName))
	{
		LOG_ERROR("ConfigBase::get_section(): There is no section {} in config file {}", sectionName, _configPath)
		return Section("UndefinedSection");
	}

	Section section(_config[sectionName]);
	return section;
}

Option Config::get_option(const std::string& sectionName, const std::string& optionName)
{
	if (!check_section(sectionName))
	{
		LOG_ERROR("ConfigBase::get_section(): There is no section {} in config file {}", sectionName, _configPath)
		return inicpp::option("UndefinedOption");
	}

	if (!check_option(sectionName, optionName))
	{
		LOG_ERROR("ConfigBase::get_section(): There is no section {} in config file {}", sectionName, _configPath)
		return inicpp::option("UndefinedOption");
	}

	return _config[sectionName][optionName];
}

void Config::set_section(Section& section)
{
	if (check_section(section.get_name()))
	{
		LOG_WARNING("ConfigBase::set_section(): Config {} has section {}. This section will be overwritten", _configPath, section.get_name());
		_config.remove_section(section.get_name());
	}

	_config.add_section(section._section);
}

Config::ConfigIterator Config::begin()
{
	return ConfigIterator(_config.begin());
}

Config::ConfigIterator Config::end()
{
	return ConfigIterator(_config.end());
}
