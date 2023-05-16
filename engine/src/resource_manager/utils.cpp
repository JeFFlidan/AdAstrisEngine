#include "utils.h"

using namespace ad_astris::resource;

std::string Utils::get_str_resource_type(ResourceType type)
{
	switch (type)
	{
		case ResourceType::MODEL:
			return "model";
		case ResourceType::LEVEL:
			return "scene";
		case ResourceType::TEXTURE:
			return "texture";
		case ResourceType::MATERIAL:
			return "material";
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
}