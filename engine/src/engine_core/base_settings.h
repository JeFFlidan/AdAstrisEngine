#pragma once

#include "subsettings.h"
#include "file_system/file_system.h"
#include "core/config_base.h"
#include "core/reflection.h"

#include <functional>

namespace ad_astris::ecore
{
#define SETUP_SETTINGS_LOG_METHODS(x) \
	virtual void throw_fatal_message(const std::string& additionalInfo)\
	{\
		LOG_FATAL("{}::get_subsettings(): No subsettings {}", #x, additionalInfo)\
	}

#define SETUP_BUILTIN_SUBSETTINGS(...) \
	template<typename ...ARGS>\
	void create_subsettings_builder_internal()\
	{\
		((create_subsettings_builder<ARGS>()), ...);\
	}\
	virtual void create_builders_for_subsettings() override \
	{\
		create_subsettings_builder_internal<__VA_ARGS__>();\
	}

	template<typename T>
	struct DefaultSettingsContext
	{
		
	};
	
	class BaseSettings
	{
		public:
			virtual ~BaseSettings() { }
			void serialize(io::FileSystem* fileSystem);
			void deserialize(const io::URI& engineConfigFilePath);

			template<typename T>
			T* get_subsettings()
			{
				uint64_t hash = T::get_type_id_static();
				auto it = _subsettingsByItsHash.find(hash);
				if (it == _subsettingsByItsHash.end())
				{
					throw_fatal_message(get_type_name<T>());
				}
				return static_cast<T*>(it->second.get());
			}

			template<typename T>
			void add_new_subsettings()
			{
				add_new_subsettings_common<T>();
			}

			template<typename T>
			void add_new_subsettings(T& subsettings)
			{
				if (add_new_subsettings_common<T>())
				{
					T* newSubsettings = static_cast<T*>(_subsettingsByItsHash[T::get_type_id_static()].get());
					*newSubsettings = subsettings;
				}
			}

			template<typename T>
			T* deserialize_custom_subsettings()
			{
				std::string subsettingsName = get_type_name<T>();
				auto it = _customSubsettingsConfigByName.find(subsettingsName);
				if (it == _customSubsettingsConfigByName.end())
				{
					throw_fatal_message(subsettingsName);
				}

				Config& config = it->second;
				Section section = config.get_section(subsettingsName);
				std::unique_ptr<T> subsettingsObject(new T());
				subsettingsObject->deserialize(section);

				uint64_t hash = subsettingsObject->get_type_id();
				_subsettingsByItsHash[hash] = std::move(subsettingsObject);
				_customSubsettingsConfigByName.erase(it);
				return static_cast<T*>(_subsettingsByItsHash[hash].get());
			}
		
		protected:
			using SubsettingsBuilder = std::function<uint64_t(std::unordered_map<uint64_t, std::unique_ptr<ISubsettings>>&)>;
		
			Config _config;
			std::unordered_map<uint64_t, std::unique_ptr<ISubsettings>> _subsettingsByItsHash;
			std::unordered_map<std::string, Config> _customSubsettingsConfigByName;
			std::unordered_map<std::string, SubsettingsBuilder> _builtinSubsettingsBuildersByName;

			template<typename T>
			bool add_new_subsettings_common()
			{
				uint64_t hash = T::get_type_id_static();
				auto it1 = _subsettingsByItsHash.find(hash);
				auto it2 = _customSubsettingsConfigByName.find(get_type_name<T>());
				if (it1 != _subsettingsByItsHash.end() || it2 != _customSubsettingsConfigByName.end())
				{
					return false;
				}
				std::unique_ptr<ISubsettings> subsettings(new T());
				_subsettingsByItsHash[hash] = std::move(subsettings);

				return true;
			}

			template<typename T>
			void create_subsettings_builder()
			{
				SubsettingsBuilder func = [](std::unordered_map<uint64_t, std::unique_ptr<ISubsettings>>& subsettingsMap)->uint64_t
				{
					std::unique_ptr<T> typedSubsettings(new T());
					uint64_t hash = typedSubsettings->get_type_id();
					subsettingsMap[hash] = std::move(typedSubsettings);
					return hash;
				};

				_builtinSubsettingsBuildersByName[get_type_name<T>()] = func;
			}

			virtual void create_builders_for_subsettings() = 0;
			virtual bool deserialize_builtin_subsettings(Section& section);
			void add_custom_subsettings_config_to_map(Section& customSubsettings);
			virtual void throw_fatal_message(const std::string& additionalInfo);
	};
}