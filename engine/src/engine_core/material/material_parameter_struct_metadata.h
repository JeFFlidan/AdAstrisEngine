#pragma once

#include "material_parameter_struct.h"
#include "material_parameter_metadata.h"
#include "file_system/file_system.h"
#include "core/custom_objects_to_json.h"

namespace ad_astris::ecore
{
	class MaterialParameterStructMetadata
	{
		friend void to_json(nlohmann::json& j, const MaterialParameterStructMetadata& metadata);
		friend void from_json(const nlohmann::json& j, MaterialParameterStructMetadata& metadata);
		
		public:
			MaterialParameterStructMetadata() = default;
			MaterialParameterStructMetadata(const io::URI& parameterStructPath, const nlohmann::json& materialMetadata);

			MaterialParameterStruct* create_struct();
			void update(const io::URI& newMaterialFolderPath, const nlohmann::json& materialMetadata);

			const io::URI& get_material_folder_path() const { return _materialFolderPath; }
		
		private:
			io::URI _materialFolderPath;
			std::unordered_map<std::string, uint32_t> _parameterIndexByName;
			std::vector<std::unique_ptr<MaterialParameterMetadata>> _parameterMetadatas;
			std::vector<std::unique_ptr<MaterialParameterStruct>> _createdStructs;

			std::string get_struct_fields_data(const std::string& paramStructData) const;
			void read_material_header(std::string& outParamStructData) const;
			std::string get_field_name(const std::string& structLine) const;
	};

	inline void to_json(nlohmann::json& j, const MaterialParameterStructMetadata& metadata)
	{
		std::vector<nlohmann::json> parameterMetadataJsons;
		for (auto& parameterMetadata : metadata._parameterMetadatas)
		{
			parameterMetadataJsons.push_back(*parameterMetadata);
		}
		j["parameter_metadatas"] = parameterMetadataJsons;
		j["parameter_struct_path"] = metadata._materialFolderPath.string();
	}

	inline void from_json(const nlohmann::json& j, MaterialParameterStructMetadata& metadata)
	{
		std::vector<nlohmann::json> parameterMetadataJsons;
		j["parameter_metadatas"].get_to(parameterMetadataJsons);
		for (auto& parameterMetadataJson : parameterMetadataJsons)
		{
			MaterialParameterMetadata& paramMetadata = *metadata._parameterMetadatas.emplace_back(new MaterialParameterMetadata());
			parameterMetadataJson.get_to(paramMetadata);
			metadata._parameterIndexByName[paramMetadata.get_name()] = metadata._parameterMetadatas.size() - 1; 
		}
		metadata._materialFolderPath = j["parameter_struct_path"].get<std::string>();
	}
}