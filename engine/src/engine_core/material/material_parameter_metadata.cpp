﻿#include "material_parameter_metadata.h"
#include "core/custom_objects_to_json.h"

using namespace ad_astris;
using namespace ecore;

constexpr const char* PARAMETER_NAME_KEY = "parameter_name";
constexpr const char* OFFSET_KEY = "offset";
constexpr const char* TYPE_KEY = "type";
constexpr const char* PRECISION_KEY = "precision";

MaterialParameterValueSerializer PARAMETER_VALUE_SERIALIZER;

nlohmann::json MaterialParameterMetadata::serialize() const
{
	nlohmann::json metadata;
	metadata[PARAMETER_NAME_KEY] = _name;
	metadata[OFFSET_KEY] = _offset;
	metadata[TYPE_KEY] = (int)_type;
	metadata[PRECISION_KEY] = (int)_precision;
	PARAMETER_VALUE_SERIALIZER.serialize(metadata, _defaultValue);
	return metadata;
}

void MaterialParameterMetadata::deserialize(const nlohmann::json& metadata)
{
	_name = metadata[PARAMETER_NAME_KEY];
	_offset = metadata[OFFSET_KEY];
	_type = (MaterialParameterType)metadata[TYPE_KEY];
	_precision = (MaterialParameterPrecision)metadata[PRECISION_KEY];
	PARAMETER_VALUE_SERIALIZER.deserialie(metadata, _defaultValue, (int)_type);
}
