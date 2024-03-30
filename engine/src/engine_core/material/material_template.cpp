#include "material_template.h"
#include "material_parameter_struct_metadata.h"
#include "material_parameter_struct_definitions.h"
#include "rhi/utils.h"
#include "core/constants.h"

using namespace ad_astris;
using namespace ecore;

constexpr const char* UUID_KEY = "uuid";
constexpr const char* DOMAIN_KEY = "domain";
constexpr const char* BLEND_MODE_KEY = "blend_mode";
constexpr const char* SHADING_MODEL_KEY = "shading_model";
constexpr const char* SHADER_PATHS_KEY = "shader_paths";
constexpr const char* PARAM_STRUCT_METADATA_KEY = "param_struct_metadata";

MaterialTemplate::MaterialTemplate(const MaterialTemplateCreateInfo& createInfo, ObjectName* name)
{
	_name = name;
	nlohmann::json materialMetadata;
	read_material_metadata(createInfo.materialDataFolderPath, materialMetadata);
	get_shader_paths(materialMetadata);
	_info.parameterStructMetadata = std::make_unique<MaterialParameterStructMetadata>(
		createInfo.materialDataFolderPath,
		materialMetadata
	);
}

MaterialParameterStruct* MaterialTemplate::create_parameter_struct() const
{
	return _info.parameterStructMetadata->create_struct();
}

void MaterialTemplate::update(const MaterialTemplateUpdateInfo& updateInfo)
{
	assert(
		updateInfo.domain != MaterialDomain::UNDEFINED &&
		updateInfo.blendMode != MaterialBlendMode::UNDEFINED &&
		updateInfo.shadingModel != MaterialShadingModel::UNDEFINED
	);

	_info.domain = updateInfo.domain;
	_info.blendMode = updateInfo.blendMode;
	_info.shadingModel = updateInfo.shadingModel;

	nlohmann::json materialMetadata;
	read_material_metadata(updateInfo.materialDataFolderPath, materialMetadata);
	get_shader_paths(materialMetadata);
	_info.parameterStructMetadata->update(updateInfo.materialDataFolderPath, materialMetadata);
}

void MaterialTemplate::serialize(io::File* file)
{
	nlohmann::json metadata;
	metadata[UUID_KEY] = _uuid;
	metadata[DOMAIN_KEY] = _info.domain;
	metadata[BLEND_MODE_KEY] = _info.blendMode;
	metadata[SHADING_MODEL_KEY] = _info.shadingModel;
	metadata[PARAM_STRUCT_METADATA_KEY] = *_info.parameterStructMetadata;

	nlohmann::json shaderPathsJson;
	for (auto& pair : _info.shaderPathByType)
		shaderPathsJson[rhi::to_string(pair.first)] = pair.second.string();
	metadata[SHADER_PATHS_KEY] = shaderPathsJson;

	file->set_metadata(metadata.dump(JSON_INDENT));
}

void MaterialTemplate::deserialize(io::File* file, ObjectName* objectName)
{
	_path = file->get_file_path();
	_name = objectName;
	
	nlohmann::json metadata = nlohmann::json::parse(file->get_metadata());
	_uuid = metadata[UUID_KEY];
	_info.domain = metadata[DOMAIN_KEY];
	_info.blendMode = metadata[BLEND_MODE_KEY];
	_info.shadingModel = metadata[SHADING_MODEL_KEY];
	
	get_shader_paths(metadata);
	
	_info.parameterStructMetadata = std::make_unique<MaterialParameterStructMetadata>();
	metadata[PARAM_STRUCT_METADATA_KEY].get_to(*_info.parameterStructMetadata);
}

void MaterialTemplate::read_material_metadata(const io::URI& materialFolderPath, nlohmann::json& outMetadata) const
{
	std::string paramStructMetadata;
	const std::string materialName = materialFolderPath.string().substr(materialFolderPath.string().find_last_of("/\\") + 1);
	io::Utils::read_file(FILE_SYSTEM(), materialFolderPath + "/" + materialName + MATERIAL_STRUCT_METADATA_EXTENSION, paramStructMetadata);

	outMetadata = nlohmann::json::parse(paramStructMetadata);
}

void MaterialTemplate::get_shader_paths(const nlohmann::json& metadata)
{
	nlohmann::json shaderPathsJson = metadata[SHADER_PATHS_KEY];
	_info.shaderPathByType.clear();
	for (auto& items : shaderPathsJson.items())
	{
		rhi::ShaderType shaderType;
		rhi::from_string(items.key(), shaderType);
		_info.shaderPathByType[shaderType] = io::URI(items.value());
	}
}
