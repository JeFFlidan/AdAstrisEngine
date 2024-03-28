#include "material_parameter_struct_metadata.h"

using namespace ad_astris;
using namespace ecore;

MaterialParameterStruct::MaterialParameterStruct(MaterialParameterStructMetadata* metadata)
	: _metadata(metadata)
{
	
}

void MaterialParameterStruct::add_parameter(MaterialParameterMetadata* parameterMetadata)
{
	
}

void MaterialParameterStruct::remove_parameter(MaterialParameterMetadata* parameterMetadata)
{
	
}

MaterialParameterStructMetadata* MaterialParameterStruct::get_metadata() const
{
	return _metadata;
}
