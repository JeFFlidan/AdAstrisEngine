#include "material_common.h"
#include <json.hpp>

#include "shader.h"
#include "shader_pass.h"

using namespace ad_astris;
using namespace ecore;
using namespace material;

std::string Utils::pack_general_material_template_info(GeneralMaterialTemplateInfo& info)
{
	nlohmann::json templateMetadata;
	templateMetadata["passes_order"] = info.shaderPassesOrder;
	templateMetadata["uuid"] = (uint64_t)info.uuid;

	for (auto& pair : info.shaderPassByItsName)
	{
		templateMetadata[pair.first] = pair.second.serialize();
	}

	return templateMetadata.dump();
}

GeneralMaterialTemplateInfo Utils::unpack_general_material_template_info(std::string& metadata)
{
	nlohmann::json jsonMetadata = nlohmann::json::parse(metadata);

	GeneralMaterialTemplateInfo templateInfo;

	for (auto& keyAndValue : jsonMetadata.items())
	{
		if (keyAndValue.key() == "uuid")
		{
			templateInfo.uuid = UUID(keyAndValue.value().get<uint64_t>());
		}
		else if (keyAndValue.key() == "passes_order")
		{
			templateInfo.shaderPassesOrder = keyAndValue.value().get<std::vector<std::string>>();
		}
		else
		{
			ShaderPass shaderPass(keyAndValue.key());
			std::string shaderPassMetadata = keyAndValue.value().get<std::string>();
			shaderPass.deserialize(shaderPassMetadata);
			templateInfo.shaderPassByItsName[shaderPass.get_name()] = shaderPass;
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
		shaderPassInfo.shaderUUIDContext.shaderUUIDs.push_back(UUID(shaderInfo.value()));
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
