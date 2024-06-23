#include "material_parameter_struct_metadata.h"
#include "material_parameter_struct_definitions.h"
#include "core/global_objects.h"
#include <sstream>

using namespace ad_astris;
using namespace ecore;

MaterialParameterStructMetadata::MaterialParameterStructMetadata(const io::URI& parameterStructPath, const nlohmann::json& materialMetadata)
	: _materialFolderPath(parameterStructPath)
{
	std::string paramStructData;
	read_material_header(paramStructData);
	
	std::string line;
	uint32_t offset = 0;
	std::stringstream paramStructFieldDataStream(get_struct_fields_data(paramStructData));
	while (std::getline(paramStructFieldDataStream, line))
	{
		std::string fieldName = get_field_name(line);
		if (materialMetadata.contains(fieldName))
		{
			_parameterIndexByName[fieldName] = (uint32_t)_parameterMetadatas.size();
			MaterialParameterMetadata& paramMetadata = *_parameterMetadatas.emplace_back(new MaterialParameterMetadata(
				fieldName,
				offset,
				materialMetadata[fieldName],
				MaterialParameterPrecision::UNDEFINED
			));
			offset += paramMetadata.get_size();
		}
	}
}

MaterialParameterStruct* MaterialParameterStructMetadata::create_struct()
{
	MaterialParameterStruct* paramStruct = _createdStructs.emplace_back(new MaterialParameterStruct(this)).get();
	for (auto& paramMetadata : _parameterMetadatas)
	{
		paramStruct->add_parameter(paramMetadata.get());
	}
	return paramStruct;
}

void MaterialParameterStructMetadata::update(const io::URI& newMaterialFolderPath, const nlohmann::json& materialMetadata)
{
	_materialFolderPath = newMaterialFolderPath;

	std::string paramStructData;
	read_material_header(paramStructData);

	std::string line;
	uint32_t offset = 0;
	size_t fieldCount = 0;
	std::vector<MaterialParameterMetadata*> updatedMaterialParamMetadatas;
	std::stringstream paramStructFieldDataStream(get_struct_fields_data(paramStructData));
	
	while (std::getline(paramStructFieldDataStream, line))
	{
		std::string fieldName = get_field_name(line);
		if (!materialMetadata.contains(fieldName))
			continue;

		MaterialParameterValue newDefaultValue = materialMetadata[fieldName];
		if (fieldCount < _parameterMetadatas.size())
		{
			MaterialParameterMetadata& paramMetadata = *_parameterMetadatas[fieldCount];
			if (paramMetadata.get_type() == (MaterialParameterType)newDefaultValue.index())
			{
				if (paramMetadata.get_name() != fieldName)
				{
					_parameterIndexByName.erase(paramMetadata.get_name());
					_parameterIndexByName[fieldName] = fieldCount;
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
				_parameterIndexByName.erase(paramMetadata.get_name());
				paramMetadata = MaterialParameterMetadata(
					fieldName,
					offset,
					newDefaultValue,
					MaterialParameterPrecision::UNDEFINED
				);
				_parameterIndexByName[fieldName] = fieldCount;
				updatedMaterialParamMetadatas.push_back(&paramMetadata);
			}
		}
		else
		{
			MaterialParameterMetadata& paramMetadata = *_parameterMetadatas.emplace_back(new MaterialParameterMetadata(
				fieldName,
				offset,
				newDefaultValue,
				MaterialParameterPrecision::UNDEFINED
			));
			_parameterIndexByName[paramMetadata.get_name()] = fieldCount;
			updatedMaterialParamMetadatas.push_back(&paramMetadata);
		}
		
		offset += _parameterMetadatas[fieldCount]->get_size();
		++fieldCount;
	}

	if (fieldCount < _parameterMetadatas.size())
	{
		size_t count = _parameterMetadatas.size() - fieldCount;
		auto rangeBegin = _parameterMetadatas.begin() + fieldCount;
		auto rangeEnd = _parameterMetadatas.begin() + fieldCount + count;
		_parameterMetadatas.erase(rangeBegin, rangeEnd);
	}

	for (auto& paramStruct : _createdStructs)
	{
		uint32_t paramCount = (uint32_t)paramStruct->get_parameters().size(); 
		for (auto& paramMetadata : updatedMaterialParamMetadatas)
		{
			auto it = _parameterIndexByName.find(paramMetadata->get_name());
			assert(it != _parameterIndexByName.end());
			if (it->second < paramCount)
				paramStruct->reset_parameter_to_default_value(it->second);
			else
				paramStruct->add_parameter(paramMetadata);
		}
		if (fieldCount < paramCount)
			paramStruct->remove_parameters(fieldCount, paramCount - fieldCount);
	}
}

std::string MaterialParameterStructMetadata::get_struct_fields_data(const std::string& paramStructData) const
{
	size_t startPos = paramStructData.find(IFDEF_MATERIAL_PARAMETERS_FLAG) + strlen(IFDEF_MATERIAL_PARAMETERS_FLAG);
	size_t endPos = paramStructData.find(ENDIF_MATERIAL_PARAMETERS_FLAG);
	return paramStructData.substr(startPos, endPos - startPos);
}

void MaterialParameterStructMetadata::read_material_header(std::string& outParamStructData) const
{
	const std::string& path = _materialFolderPath.string(); 
	const std::string materialName = path.substr(path.find_last_of("/\\") + 1);
	
	io::Utils::read_file(FILE_SYSTEM(), path + "/" + materialName + ".h", outParamStructData);
}

std::string MaterialParameterStructMetadata::get_field_name(const std::string& structLine) const
{
	size_t startPos = structLine.find_last_of(" ");
	size_t endPos = structLine.find(";") - 1;
	return structLine.substr(startPos + 1, endPos - startPos);
}
