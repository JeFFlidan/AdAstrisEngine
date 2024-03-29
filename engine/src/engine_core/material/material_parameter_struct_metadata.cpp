#include "material_parameter_struct_metadata.h"
#include "material_parameter_struct_metadata_creator.h"

using namespace ad_astris;
using namespace ecore;

MaterialParameterStructMetadata::MaterialParameterStructMetadata(const io::URI& parameterStructPath)
	: _materialFolderPath(parameterStructPath)
{
	
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

void MaterialParameterStructMetadata::update(const io::URI& newMaterialFolderPath)
{
	_materialFolderPath = newMaterialFolderPath;
	MaterialParameterStructMetadataCreator::update(this);
}
