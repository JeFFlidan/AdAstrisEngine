#include "material_parameter_struct_metadata.h"

using namespace ad_astris;
using namespace ecore;

MaterialParameterStructMetadata::MaterialParameterStructMetadata(const io::URI& parameterStructPath)
	: _parameterStructPath(parameterStructPath)
{
	
}

MaterialParameterStruct* MaterialParameterStructMetadata::create_struct()
{
	MaterialParameterStruct* paramStruct = _structs.emplace_back(new MaterialParameterStruct(this)).get();
	for (auto& paramMetadata : _parameterMetadatas)
	{
		paramStruct->add_parameter(paramMetadata.get());
	}
	return paramStruct;
}

void MaterialParameterStructMetadata::update(const io::URI& newParameterStructPath)
{
	
}
