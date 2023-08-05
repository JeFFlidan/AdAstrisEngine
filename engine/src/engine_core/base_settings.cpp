#include "base_settings.h"

using namespace ad_astris;
using namespace ecore;

void BaseSettings::serialize(io::FileSystem* fileSystem)
{
	for (auto& pair : _subsettingsByItsHash)
	{
		pair.second.get()->serialize(_config);
	}

	for (auto& pair : _customSubsettingsConfigByName)
	{
		Section section = pair.second.get_section(pair.first);
		_config.set_section(section);
	}

	_config.save(fileSystem);
}
		
void BaseSettings::deserialize(const io::URI& engineConfigFilePath)
{
	_config.load_from_file(engineConfigFilePath);
	create_builders_for_subsettings();

	for (auto section : _config)
	{
		if (!deserialize_builtin_subsettings(section))
		{
			add_custom_subsettings_config_to_map(section);
		}
	}
	
	_config.unload();
}

bool BaseSettings::deserialize_builtin_subsettings(Section& section)
{
	std::string sectionName = section.get_name();
	auto it = _builtinSubsettingsBuildersByName.find(sectionName);
	if (it == _builtinSubsettingsBuildersByName.end())
		return false;

	uint64_t hash = it->second(_subsettingsByItsHash);
	_subsettingsByItsHash[hash]->deserialize(section);
	_builtinSubsettingsBuildersByName.erase(it);
	return true;
}

void BaseSettings::add_custom_subsettings_config_to_map(Section& customSubsettings)
{
	Config newConfig;
	newConfig.set_section(customSubsettings);
	_customSubsettingsConfigByName[customSubsettings.get_name()] = newConfig;
}

void BaseSettings::throw_fatal_message(const std::string& additionalInfo)
{
	LOG_FATAL("BaseSettings::get_subsettings(): No subsettings {}", additionalInfo)
}
