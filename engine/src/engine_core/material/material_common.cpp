#include "material_common.h"
#include <json/json.hpp>

#include "shader.h"
#include "shader_pass.h"

using namespace ad_astris;
using namespace ecore;
using namespace material;

std::string Utils::pack_general_material_template_info(MaterialTemplateInfo& info)
{
	nlohmann::json templateMetadata;
	templateMetadata["uuid"] = (uint64_t)info.uuid;
	templateMetadata["material_domain"] = get_str_material_domain(info.domain);
	templateMetadata["material_blend_mode"] = get_str_material_blend_mode(info.blendMode);
	templateMetadata["material_shading_model"] = get_str_material_shading_model(info.shadingModel);
	templateMetadata["material_type"] = get_str_material_type(info.materialType);

	for (auto& pair : info.shaderPassByItsType)
	{
		templateMetadata[get_str_shader_pass_type(pair.first)] = pair.second.serialize();
	}

	return templateMetadata.dump();
}

MaterialTemplateInfo Utils::unpack_general_material_template_info(std::string& metadata)
{
	nlohmann::json jsonMetadata = nlohmann::json::parse(metadata);

	MaterialTemplateInfo templateInfo;

	for (auto& keyAndValue : jsonMetadata.items())
	{
		if (keyAndValue.key() == "uuid")
		{
			templateInfo.uuid = UUID(keyAndValue.value().get<uint64_t>());
		}
		else if (keyAndValue.key() == "material_domain")
		{
			templateInfo.domain = get_enum_material_domain(keyAndValue.value());
		}
		else if (keyAndValue.key() == "material_blend_mode")
		{
			templateInfo.blendMode = get_enum_material_blend_mode(keyAndValue.value());
		}
		else if (keyAndValue.key() == "material_shading_model")
		{
			templateInfo.shadingModel = get_enum_material_shading_model(keyAndValue.value());
		}
		else if (keyAndValue.key() == "material_type")
		{
			templateInfo.materialType = get_enum_material_type(keyAndValue.value());
		}
		else
		{
			ShaderPass shaderPass(get_enum_shader_pass_type(keyAndValue.key()));
			std::string shaderPassMetadata = keyAndValue.value().get<std::string>();
			shaderPass.deserialize(shaderPassMetadata);
			templateInfo.shaderPassByItsType[shaderPass.get_type()] = shaderPass;
		}
	}

	return templateInfo;
}

std::string Utils::pack_shader_pass_info(ShaderPassInfo& info)
{
	nlohmann::json shaderPassMetadata;

	std::vector<ShaderHandle> shaderHandles;
	info.shaderHandleContext.get_all_valid_shader_handles(shaderHandles);
	for (auto& shaderHandle : shaderHandles)
	{
		Shader* shader = shaderHandle.get_resource();
		shaderPassMetadata[shader->get_name()->get_full_name()] = (uint64_t)shader->get_uuid();
	}
	return shaderPassMetadata.dump();
}

ShaderPassInfo Utils::unpack_shader_pass_info(std::string& metadata)
{
	nlohmann::json shaderPassMetadata = nlohmann::json::parse(metadata);

	ShaderPassInfo shaderPassInfo;
	for (auto& shaderInfo : shaderPassMetadata.items())
	{
		shaderPassInfo.shaderUUIDContext.shaderUUIDs.push_back(UUID((uint64_t)shaderInfo.value()));
	}

	return shaderPassInfo;
}

void ShaderHandleContext::get_all_valid_shader_handles(std::vector<ShaderHandle>& shaderHandles)
{
	if (vertexShader.is_valid())
	{
		shaderHandles.push_back(vertexShader);
	}
	if (fragmentShader.is_valid())
	{
		shaderHandles.push_back(fragmentShader);
	}
	if (tessControlShader.is_valid())
	{
		shaderHandles.push_back(tessControlShader);
	}
	if (tessEvaluationShader.is_valid())
	{
		shaderHandles.push_back(tessEvaluationShader);
	}
	if (geometryShader.is_valid())
	{
		shaderHandles.push_back(geometryShader);
	}
	if (computeShader.is_valid())
	{
		shaderHandles.push_back(computeShader);
	}
	if (meshShader.is_valid())
	{
		shaderHandles.push_back(meshShader);
	}
	if (taskShader.is_valid())
	{
		shaderHandles.push_back(taskShader);
	}
	if (rayGenerationShader.is_valid())
	{
		shaderHandles.push_back(rayGenerationShader);
	}
	if (rayIntersectionShader.is_valid())
	{
		shaderHandles.push_back(rayIntersectionShader);
	}
	if (rayAnyHitShader.is_valid())
	{
		shaderHandles.push_back(rayAnyHitShader);
	}
	if (rayClosestHitShader.is_valid())
	{
		shaderHandles.push_back(rayClosestHitShader);
	}
	if (rayMissShader.is_valid())
	{
		shaderHandles.push_back(rayMissShader);
	}
	if (rayCallableShader.is_valid())
	{
		shaderHandles.push_back(rayCallableShader);
	}
}

std::string Utils::get_str_shader_pass_type(ShaderPassType shaderPassType)
{
	switch (shaderPassType)
	{
		case ShaderPassType::GBUFFER:
			return "gbuffer";
		case ShaderPassType::DEFERRED_LIGHTING:
			return "deferred_lighting";
		case ShaderPassType::OIT_PREPASS:
			return "oit_prepass";
		case ShaderPassType::OIT:
			return "oit";
		case ShaderPassType::TAA:
			return "taa";
		case ShaderPassType::DIRECTIONAL_LIGHT_SHADOWS:
			return "directional_light_shadows";
		case ShaderPassType::POINT_LIGHT_SHADOWS:
			return "point_light_shadows";
		case ShaderPassType::SPOT_LIGHT_SHADOWS:
			return "spot_light_shadows";
		case ShaderPassType::COMPOSITE:
			return "composite";
		case ShaderPassType::POSTPROCESSING:
			return "postprocessing";
		case ShaderPassType::CULLING:
			return "culling";
		case ShaderPassType::REDUCE_DEPTH:
			return "reduce_depth";
	}
}

ShaderPassType Utils::get_enum_shader_pass_type(const std::string& shaderPassType)
{
	if (shaderPassType == "gbuffer")
		return ShaderPassType::GBUFFER;
	if (shaderPassType == "deferred_lighting")
		return ShaderPassType::DEFERRED_LIGHTING;
	if (shaderPassType == "oit_prepass")
		return ShaderPassType::OIT_PREPASS;
	if (shaderPassType == "oit")
		return ShaderPassType::OIT;
	if (shaderPassType == "taa")
		return ShaderPassType::TAA;
	if (shaderPassType == "directional_light_shadows")
		return ShaderPassType::DIRECTIONAL_LIGHT_SHADOWS;
	if (shaderPassType == "point_light_shadows")
		return ShaderPassType::POINT_LIGHT_SHADOWS;
	if (shaderPassType == "spot_light_shadows")
		return ShaderPassType::SPOT_LIGHT_SHADOWS;
	if (shaderPassType == "composite")
		return ShaderPassType::COMPOSITE;
	if (shaderPassType == "postprocessing")
		return ShaderPassType::POSTPROCESSING;
	if (shaderPassType == "culling")
		return ShaderPassType::CULLING;
	if (shaderPassType == "reduce_depth")
		return ShaderPassType::REDUCE_DEPTH;
}

std::string Utils::get_str_material_domain(MaterialDomain materialDomain)
{
	switch (materialDomain)
	{
		case MaterialDomain::UNDEFINED:
			return "undefined";
		case MaterialDomain::SURFACE:
			return "surface";
		case MaterialDomain::DEFERRED_DECAL:
			return "deferred_decal";
		case MaterialDomain::POSTPROCESSING:
			return "postprocessing";
	}
}

MaterialDomain Utils::get_enum_material_domain(const std::string& materialDomain)
{
	if (materialDomain == "undefined")
		return MaterialDomain::UNDEFINED;
	if (materialDomain == "surface")
		return MaterialDomain::SURFACE;
	if (materialDomain == "deferred_decal")
		return MaterialDomain::DEFERRED_DECAL;
	if (materialDomain == "postprocessing")
		return MaterialDomain::POSTPROCESSING;
}

std::string Utils::get_str_material_blend_mode(MaterialBlendMode materialBlendMode)
{
	switch (materialBlendMode)
	{
		case MaterialBlendMode::UNDEFINED:
			return "undefined";
		case MaterialBlendMode::OPAQUE:
			return "opaque";
		case MaterialBlendMode::TRANSPARENT:
			return "transparent";
	}
}

MaterialBlendMode Utils::get_enum_material_blend_mode(const std::string& materialBlendMode)
{
	if (materialBlendMode == "undefined")
		return MaterialBlendMode::UNDEFINED;
	if (materialBlendMode == "opaque")
		return MaterialBlendMode::OPAQUE;
	if (materialBlendMode == "transparent")
		return MaterialBlendMode::TRANSPARENT;
}

std::string Utils::get_str_material_shading_model(MaterialShadingModel materialShadingModel)
{
	switch (materialShadingModel)
	{
		case MaterialShadingModel::UNDEFINED:
			return "undefined";
		case MaterialShadingModel::UNLIT:
			return "unlit";
		case MaterialShadingModel::DEFAULT_LIT:
			return "default_lit";
	}
}

MaterialShadingModel Utils::get_enum_material_shading_model(const std::string& materialShadingModel)
{
	if (materialShadingModel == "undefined")
		return MaterialShadingModel::UNDEFINED;
	if (materialShadingModel == "unlit")
		return MaterialShadingModel::UNLIT;
	if (materialShadingModel == "default_lit")
		return MaterialShadingModel::DEFAULT_LIT;
}

std::string Utils::get_str_material_type(MaterialType materialType)
{
	switch (materialType)
	{
		case MaterialType::GRAPHICS:
			return "graphics";
		case MaterialType::RAY_TRACING:
			return "ray_tracing";
		case MaterialType::COMPUTE:
			return "compute";
	}
}

MaterialType Utils::get_enum_material_type(const std::string& materialType)
{
	if (materialType == "graphics")
		return MaterialType::GRAPHICS;
	if (materialType == "ray_tracing")
		return MaterialType::RAY_TRACING;
	if (materialType == "compute")
		return MaterialType::COMPUTE;
}

