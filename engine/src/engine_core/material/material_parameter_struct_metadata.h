#pragma once

#include "material_parameter_struct.h"
#include "material_parameter_metadata.h"
#include "core/global_objects.h"

namespace ad_astris::ecore
{
	class MaterialParameterStructMetadataCreator;
	class MaterialParameterStructMetadata;
	
	class MaterialParameterStructMetadata
	{
		friend MaterialParameterStructMetadataCreator;
		friend void to_json(nlohmann::json& j, const MaterialParameterStructMetadata& metadata);
		friend void from_json(const nlohmann::json& j, MaterialParameterStructMetadata& metadata);
		
		public:
			MaterialParameterStructMetadata() = default;
			MaterialParameterStructMetadata(const io::URI& parameterStructPath);

			MaterialParameterStruct* create_struct();
			void update(const io::URI& newParameterStructPath);
		
		private:
			io::URI _parameterStructPath;
			std::unordered_map<std::string, uint32_t> _parameterIndexByName;
			std::vector<std::unique_ptr<MaterialParameterMetadata>> _parameterMetadatas;
			std::vector<std::unique_ptr<MaterialParameterStruct>> _structs;
	};

	inline void to_json(nlohmann::json& j, const MaterialParameterStructMetadata& metadata)
	{
		std::vector<nlohmann::json> parameterMetadataJsons;
		for (auto& parameterMetadata : metadata._parameterMetadatas)
		{
			parameterMetadataJsons.push_back(*parameterMetadata);
		}
		j["parameter_metadatas"] = parameterMetadataJsons;
		j["parameter_struct_path"] = metadata._parameterStructPath.string();
	}

	inline void from_json(const nlohmann::json& j, MaterialParameterStructMetadata& metadata)
	{
		std::vector<nlohmann::json> parameterMetadataJsons = j["parameter_metadatas"];
		for (auto& parameterMetadataJson : parameterMetadataJsons)
		{
			MaterialParameterMetadata& paramMetadata = *metadata._parameterMetadatas.emplace_back(new MaterialParameterMetadata());
			parameterMetadataJson.get_to(paramMetadata);
			metadata._parameterIndexByName[paramMetadata.get_name()] = metadata._parameterMetadatas.size() - 1; 
		}
		metadata._parameterStructPath = j["parameter_struct_path"].get<std::string>();
	}
}