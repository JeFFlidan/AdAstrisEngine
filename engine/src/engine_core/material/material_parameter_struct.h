#pragma once

#include "material_parameter.h"

namespace ad_astris::ecore
{
	class MaterialParameterStructMetadata;
	
	class MaterialParameterStruct
	{
		friend void to_json(nlohmann::json& j, const MaterialParameterStruct& paramStruct);
		friend void from_json(const nlohmann::json& j, MaterialParameterStruct& paramStruct);
		
		public:
			MaterialParameterStruct(MaterialParameterStructMetadata* metadata);

			void add_parameter(MaterialParameterMetadata* parameterMetadata);
			void reset_parameter_to_default_value(uint32_t parameterIndex);
			void remove_parameter(uint32_t parameterIndex);
			void remove_parameters(uint32_t rangeBeginIndex, uint32_t count);

			void copy_to_material_buffer(void* materialBufferPtr) const;
		
			MaterialParameterStructMetadata* get_metadata() const;
			const std::vector<std::unique_ptr<MaterialParameter>>& get_parameters() const { return _parameters; }

		private:
			MaterialParameterStructMetadata* _metadata{ nullptr };
			std::vector<std::unique_ptr<MaterialParameter>> _parameters;
	};

	inline void to_json(nlohmann::json& j, const MaterialParameterStruct& paramStruct)
	{
		std::vector<nlohmann::json> parameterJsons;
		for (auto& param : paramStruct._parameters)
		{
			parameterJsons.push_back(*param);
		}
		j["parameters"] = parameterJsons;
	}

	inline void from_json(const nlohmann::json& j, MaterialParameterStruct& paramStruct)
	{
		std::vector<nlohmann::json> parameterJsons;
		j["parameters"].get_to(parameterJsons);
		size_t index = 0;
		for (auto& parameterJson : parameterJsons)
		{
			parameterJson.get_to(*paramStruct._parameters[index++]);
		}
	}
}