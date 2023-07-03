#include "utils.h"

using namespace ad_astris::resource;

std::string Utils::get_str_resource_type(ResourceType type)
{
	switch (type)
	{
		case ResourceType::MODEL:
			return "model";
		case ResourceType::LEVEL:
			return "level";
		case ResourceType::TEXTURE:
			return "texture";
		case ResourceType::MATERIAL:
			return "material";
		case ResourceType::MATERIAL_TEMPLATE:
			return "material_template";
		case ResourceType::SHADER:
			return "shader";
	}
}

ResourceType Utils::get_enum_resource_type(std::string type)
{
	if (type == "model")
		return ResourceType::MODEL;
	if (type == "texture")
		return ResourceType::TEXTURE;
	if (type == "level")
		return ResourceType::LEVEL;
	if (type == "material")
		return ResourceType::MATERIAL;
	if (type == "material_template")
		return ResourceType::MATERIAL_TEMPLATE;
	if (type == "shader")
		return ResourceType::SHADER;
}