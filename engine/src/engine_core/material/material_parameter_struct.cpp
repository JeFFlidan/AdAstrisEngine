#include "material_parameter_struct_metadata.h"

using namespace ad_astris;
using namespace ecore;

MaterialParameterStruct::MaterialParameterStruct(MaterialParameterStructMetadata* metadata)
	: _metadata(metadata)
{
	
}

void MaterialParameterStruct::add_parameter(MaterialParameterMetadata* parameterMetadata)
{
	_parameters.emplace_back(new MaterialParameter(parameterMetadata));
}

void MaterialParameterStruct::reset_parameter_to_default_value(uint32_t parameterIndex)
{
	assert(parameterIndex < _parameters.size());
	_parameters[parameterIndex]->reset_to_default_value();
}

void MaterialParameterStruct::remove_parameter(uint32_t parameterIndex)
{
	assert(parameterIndex < _parameters.size());
	_parameters.erase(_parameters.begin() + parameterIndex);
}

void MaterialParameterStruct::remove_parameters(uint32_t rangeBeginIndex, uint32_t count)
{
	assert(rangeBeginIndex + count <= _parameters.size());
	_parameters.erase(_parameters.begin() + rangeBeginIndex, _parameters.begin() + rangeBeginIndex + count);
}

void MaterialParameterStruct::copy_to_material_buffer(void* materialBufferPtr) const
{
	uint8_t* materialBufferTypedPtr = static_cast<uint8_t*>(materialBufferPtr);
	MaterialParameterMetadata* parameterMetadata{ nullptr };
	for (auto& param : _parameters)
	{
		memcpy(materialBufferTypedPtr + parameterMetadata->get_offset(), param->get_ptr(), parameterMetadata->get_size());
	}
}

MaterialParameterStructMetadata* MaterialParameterStruct::get_metadata() const
{
	return _metadata;
}
