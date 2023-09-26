#include "material_common.h"
#include <json/json.hpp>

#include "shader.h"

using namespace ad_astris;
using namespace ecore;
using namespace material;

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

