#include "material_parameter_struct_metadata_creator.h"
#include "material_parameter_struct_definitions.h"

using namespace ad_astris;
using namespace ecore;

std::unique_ptr<MaterialParameterStructMetadata> MaterialParameterStructMetadataCreator::create(const io::URI& path)
{
	const std::string materialName = path.string().substr(path.string().find_last_of("/\\") + 1);
	
	std::string paramStructData;
	io::Utils::read_file(FILE_SYSTEM(), path + "/" + materialName + ".h", paramStructData);
	std::string paramStructMetadata;
	io::Utils::read_file(FILE_SYSTEM(), path + "/" + materialName + MATERIAL_STRUCT_METADATA_EXTENSION, paramStructMetadata);
	
	nlohmann::json paramStructJsonMetadata = nlohmann::json::parse(paramStructMetadata);

	size_t startPos = paramStructData.find(IFDEF_MATERIAL_STRUCT_FLAG) + strlen(IFDEF_MATERIAL_STRUCT_FLAG);
	size_t endPos = paramStructData.find(ENDIF_MATERIAL_STRUCT_FLAG);
	std::string paramStructFieldsData = paramStructData.substr(startPos, endPos - startPos);

	std::unique_ptr<MaterialParameterStructMetadata> metadata = std::make_unique<MaterialParameterStructMetadata>(path);
	std::stringstream paramStructFieldDataStream(paramStructFieldsData);
	std::string line;
	uint32_t offset = 0;
	while (std::getline(paramStructFieldDataStream, line))
	{
		startPos = line.find_last_of(" ");
		endPos = line.find(";") - 1;
		std::string fieldName = line.substr(startPos + 1, endPos - startPos);
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
	
}
