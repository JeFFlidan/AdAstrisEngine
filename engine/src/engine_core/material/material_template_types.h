#pragma once

#include "rhi/resources.h"
#include "core/reflector/enum_reflector.h"

namespace ad_astris::ecore
{
	enum class MaterialDomain
	{
		UNDEFINED = 0,
		SURFACE,
		DEFERRED_DECAL,
		POSTPROCESSING,
		VOLUME,
	};

	REFLECT_ENUM(MaterialDomain, {
		{ MaterialDomain::UNDEFINED, "undefined" },
		{ MaterialDomain::SURFACE, "surface" },
		{ MaterialDomain::DEFERRED_DECAL, "deferred_decal" },
		{ MaterialDomain::POSTPROCESSING, "postprocessing" },
		{ MaterialDomain::VOLUME, "volume" }
	})
	
	enum class MaterialShadingModel
	{
		UNDEFINED = 0,
		UNLIT,
		DEFAULT_LIT,
		SUBSURFACE,
		HAIR,
		CLOTH,
		EYE
	};
	
	REFLECT_ENUM(MaterialShadingModel, {
		{ MaterialShadingModel::UNDEFINED, "undefined" },
		{ MaterialShadingModel::UNLIT, "unlit" },
		{ MaterialShadingModel::DEFAULT_LIT, "default_lit" },
		{ MaterialShadingModel::SUBSURFACE, "subsurface" },
		{ MaterialShadingModel::HAIR, "hair" },
		{ MaterialShadingModel::CLOTH, "cloth" },
		{ MaterialShadingModel::EYE, "eye" }
	})
	
	enum class MaterialBlendMode
	{
		UNDEFINED = 0,
		OPAQUE_,
		TRANSPARENT_
	};

	REFLECT_ENUM(MaterialBlendMode, { 
		{ MaterialBlendMode::UNDEFINED, "undefined" },
		{ MaterialBlendMode::OPAQUE_, "opaque" },
		{ MaterialBlendMode::TRANSPARENT_, "transparent" }
	})
}
