#pragma once

#include "material_parameter_types.h"
#include "material_parameter_utils.h"
#include <string>

namespace ad_astris::ecore
{
	class MaterialParameterMetadata
	{
		public:
			MaterialParameterMetadata(
				const std::string& name,
				uint32_t offset,
				const MaterialParameterValue& defaultValue,
				MaterialParameterPrecision precision) : _name(name), _offset(offset), _defaultValue(defaultValue), _precision(precision)
			{
				_type = MaterialParameterTypeGetter::execute(_defaultValue);
			}

			MaterialParameterMetadata(const MaterialParameterMetadata&) = delete;
			void operator=(const MaterialParameterMetadata&) = delete;

			void set_name(const std::string& name) { _name = name; }
			void set_default_value(const MaterialParameterValue& value)
			{
				if (MaterialParameterTypeGetter::execute(value) == _type)
					_defaultValue = value;
			}

			uint32_t get_offset() const { return _offset; }
			std::string get_name() const { return _name; }
			MaterialParameterType get_type() const { return _type; }
			MaterialParameterPrecision get_precision() const { return _precision; }
			const MaterialParameterValue& get_default_value() const { return _defaultValue; }
		
		private:
			std::string _name;
			uint32_t _offset{ 0 };
			MaterialParameterValue _defaultValue;
			MaterialParameterType _type{ MaterialParameterType::UNDEFINED };
			MaterialParameterPrecision _precision{ MaterialParameterPrecision::UNDEFINED };	// Only for floats
	};
}