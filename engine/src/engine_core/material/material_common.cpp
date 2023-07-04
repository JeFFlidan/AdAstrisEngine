#include "material_common.h"
#include <json.hpp>

#include "shader.h"

using namespace ad_astris;
using namespace ecore;
using namespace material;

std::string Utils::pack_general_material_template_info(GeneralMaterialTemplateInfo& info)
{
	ShaderUUIDContext& uuidContext = info.shaderUUIDContext;

	nlohmann::json templateMetadata;
	templateMetadata["uuid"] = (uint64_t)info.uuid;
	
	std::vector<uint64_t> uuids;
	for (auto& uuid : uuidContext.shaderUUIDs)
	{
		uuids.push_back(uuid);
	}
	templateMetadata["shader_uuids"] = uuids;

	return templateMetadata.dump();
}

GeneralMaterialTemplateInfo Utils::unpack_general_material_template_info(std::string& metadata)
{
	nlohmann::json templateMetadata = nlohmann::json::parse(metadata);

	// TODO Think about optimizing getting UUIDs
	std::vector<uint64_t> tempShaderUUIDs = templateMetadata["shader_uuids"].get<std::vector<uint64_t>>();
	std::vector<UUID> shaderUUIDs;
	for (auto& uint64UUID : tempShaderUUIDs)
		shaderUUIDs.push_back(uint64UUID);
	
	UUID templateUUID = UUID(templateMetadata["uuid"]);

	GeneralMaterialTemplateInfo templateInfo;
	templateInfo.uuid = templateUUID;
	templateInfo.shaderUUIDContext.shaderUUIDs = shaderUUIDs;

	return templateInfo;
}
