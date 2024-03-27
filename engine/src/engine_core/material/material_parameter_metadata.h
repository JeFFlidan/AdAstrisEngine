#pragma once

#include "material_parameter_types.h"
#include <string>
#include <json/json.hpp>

namespace ad_astris::ecore
{
	class MaterialParameterMetadata
	{
		public:
			MaterialParameterMetadata() = default;
			MaterialParameterMetadata(
				const std::string& name,
				uint32_t offset,
				const MaterialParameterValue& defaultValue,
				MaterialParameterPrecision precision) : _name(name), _offset(offset), _defaultValue(defaultValue), _precision(precision)
			{
				_type = (MaterialParameterType)_defaultValue.index();
			}

			MaterialParameterMetadata(const MaterialParameterMetadata&) = delete;
			void operator=(const MaterialParameterMetadata&) = delete;

			nlohmann::json serialize() const;
			void deserialize(const nlohmann::json& metadata);

			void set_name(const std::string& name) { _name = name; }
			void set_precision(MaterialParameterPrecision precision) { _precision = precision; }
			void set_default_value(const MaterialParameterValue& value)
			{
				if ((MaterialParameterType)value.index() == _type)
					_defaultValue = value;
			}

			size_t get_size() const { return internal::MATERIAL_PARAMETER_TYPE_SIZES[(int)_type]; }
			uint32_t get_offset() const { return _offset; }
			std::string get_name() const { return _name; }
			MaterialParameterType get_type() const { return _type; }
			MaterialParameterPrecision get_precision() const { return _precision; }
			const MaterialParameterValue& get_default_value() const { return _defaultValue; }
		
		private:
			std::string _name;
			uint32_t _offset{ 0 };
			MaterialParameterValue _defaultValue;
			MaterialParameterType _type;
			MaterialParameterPrecision _precision{ MaterialParameterPrecision::UNDEFINED };	// Only for floats
	};
}