#include "material_base.h"

using namespace ad_astris::ecore;

MaterialBase::MaterialBase(ObjectName* materialName)
{
	_name = materialName;
}

void MaterialBase::serialize(io::File* file)
{
	nlohmann::json metadataJson;
	metadataJson["material_uuid"] = _materialUUID;
	metadataJson["material_settings"] = _materialSettings->serialize();
	std::string metadataStr = metadataJson.dump();
	file->set_metadata(metadataStr);
	_isDirty = false;
}

void MaterialBase::deserialize(io::File* file, ObjectName* objectName)
{
	std::string& metadataStr = file->get_metadata();
	nlohmann::json metadataJson = nlohmann::json::parse(metadataStr);
	_materialUUID = metadataJson["material_uuid"];
	_materialSettings->deserialize(metadataJson["material_settings"]);
	_path = file->get_file_path();
	_name = objectName;
}
