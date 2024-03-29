#include "material_parameter_struct_metadata_creator.h"
#include "material_parameter_struct_definitions.h"

using namespace ad_astris;
using namespace ecore;

std::string get_struct_fields_data(const std::string& paramStructData)
{
	size_t startPos = paramStructData.find(IFDEF_MATERIAL_PARAMETERS_FLAG) + strlen(IFDEF_MATERIAL_PARAMETERS_FLAG);
	size_t endPos = paramStructData.find(ENDIF_MATERIAL_PARAMETERS_FLAG);
	return paramStructData.substr(startPos, endPos - startPos);
}

void read_material_files(const io::URI& path, std::string& outParamStructData, nlohmann::json& outParamStructMetadata)
{
	const std::string materialName = path.string().substr(path.string().find_last_of("/\\") + 1);
	
	io::Utils::read_file(FILE_SYSTEM(), path + "/" + materialName + ".h", outParamStructData);
	std::string paramStructMetadata;
	io::Utils::read_file(FILE_SYSTEM(), path + "/" + materialName + MATERIAL_STRUCT_METADATA_EXTENSION, paramStructMetadata);
	
	outParamStructMetadata = nlohmann::json::parse(paramStructMetadata);
}

std::string get_field_name(const std::string& structLine)
{
	size_t startPos = structLine.find_last_of(" ");
	size_t endPos = structLine.find(";") - 1;
	return structLine.substr(startPos + 1, endPos - startPos);
}

std::unique_ptr<MaterialParameterStructMetadata> MaterialParameterStructMetadataCreator::create(const io::URI& path)
{
	std::string paramStructData;
	nlohmann::json paramStructJsonMetadata;
	read_material_files(path, paramStructData, paramStructJsonMetadata);

	std::unique_ptr<MaterialParameterStructMetadata> metadata = std::make_unique<MaterialParameterStructMetadata>(path);
	std::stringstream paramStructFieldDataStream(get_struct_fields_data(paramStructData));
	std::string line;
	uint32_t offset = 0;
	while (std::getline(paramStructFieldDataStream, line))
	{
		std::string fieldName = get_field_name(line);
		if (paramStructJsonMetadata.contains(fieldName))
		{
			metadata->_parameterIndexByName[fieldName] = (uint32_t)metadata->_parameterMetadatas.size();
			MaterialParameterMetadata& paramMetadata = *metadata->_parameterMetadatas.emplace_back(new MaterialParameterMetadata(
				fieldName,
				offset,
				paramStructJsonMetadata[fieldName],
				MaterialParameterPrecision::UNDEFINED
			));
			offset += paramMetadata.get_size();
		}
	}

	return metadata;
}

void MaterialParameterStructMetadataCreator::update(MaterialParameterStructMetadata* metadata)
{
	std::string paramStructData;
	nlohmann::json paramStructJsonMetadata;
	read_material_files(metadata->_materialFolderPath, paramStructData, paramStructJsonMetadata);

	std::string line;
	uint32_t offset = 0;
	size_t fieldCount = 0;
	std::vector<MaterialParameterMetadata*> updatedMaterialParamMetadatas;
	std::stringstream paramStructFieldDataStream(get_struct_fields_data(paramStructData));
	
	while (std::getline(paramStructFieldDataStream, line))
	{
		std::string fieldName = get_field_name(line);
		if (!paramStructJsonMetadata.contains(fieldName))
			continue;

		MaterialParameterValue newDefaultValue = paramStructJsonMetadata[fieldName];
		if (fieldCount < metadata->_parameterMetadatas.size())
		{
			MaterialParameterMetadata& paramMetadata = *metadata->_parameterMetadatas[fieldCount];
			if (paramMetadata.get_type() == (MaterialParameterType)newDefaultValue.index())
			{
				if (paramMetadata.get_name() != fieldName)
				{
					metadata->_parameterIndexByName.erase(paramMetadata.get_name());
					metadata->_parameterIndexByName[fieldName] = fieldCount;
				}
				paramMetadata = MaterialParameterMetadata(
					fieldName,
					offset,
					newDefaultValue,
					paramMetadata.get_precision()
				);
			}
			else
			{
				metadata->_parameterIndexByName.erase(paramMetadata.get_name());
				paramMetadata = MaterialParameterMetadata(
					fieldName,
					offset,
					newDefaultValue,
					MaterialParameterPrecision::UNDEFINED
				);
				metadata->_parameterIndexByName[fieldName] = fieldCount;
				updatedMaterialParamMetadatas.push_back(&paramMetadata);
			}
		}
		else
		{
			MaterialParameterMetadata& paramMetadata = *metadata->_parameterMetadatas.emplace_back(new MaterialParameterMetadata(
				fieldName,
				offset,
				newDefaultValue,
				MaterialParameterPrecision::UNDEFINED
			));
			metadata->_parameterIndexByName[paramMetadata.get_name()] = fieldCount;
			updatedMaterialParamMetadatas.push_back(&paramMetadata);
		}
		
		offset += metadata->_parameterMetadatas[fieldCount]->get_size();
		++fieldCount;
	}

	if (fieldCount < metadata->_parameterMetadatas.size())
	{
		size_t count = metadata->_parameterMetadatas.size() - fieldCount;
		auto rangeBegin = metadata->_parameterMetadatas.begin() + fieldCount;
		auto rangeEnd = metadata->_parameterMetadatas.begin() + fieldCount + count;
		metadata->_parameterMetadatas.erase(rangeBegin, rangeEnd);
	}

	for (auto& paramStruct : metadata->_createdStructs)
	{
		uint32_t paramCount = (uint32_t)paramStruct->get_parameters().size(); 
		for (auto& paramMetadata : updatedMaterialParamMetadatas)
		{
			auto it = metadata->_parameterIndexByName.find(paramMetadata->get_name());
			assert(it != metadata->_parameterIndexByName.end());
			if (it->second < paramCount)
				paramStruct->reset_parameter_to_default_value(it->second);
			else
				paramStruct->add_parameter(paramMetadata);
		}
		if (fieldCount < paramCount)
			paramStruct->remove_parameters(fieldCount, paramCount - fieldCount);
	}
}
