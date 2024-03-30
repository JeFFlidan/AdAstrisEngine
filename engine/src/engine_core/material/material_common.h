#pragma once

#include "core/reflection.h"
#include <string>

namespace ad_astris::ecore::material
{
	enum class MaterialDomain
	{
		UNDEFINED = 0,
		SURFACE,
		DEFERRED_DECAL,
		POSTPROCESSING
	};

	enum class MaterialBlendMode
	{
		UNDEFINED = 0,
		OPAQUE,
		TRANSPARENT
	};

	enum class MaterialShadingModel
	{
		UNDEFINED = 0,
		UNLIT,
		DEFAULT_LIT
	};

	enum class MaterialType
	{
		UNDEFINED = 0,
		GRAPHICS,
		RAY_TRACING,
		COMPUTE
	};
	
	class Utils
	{
		public:
			static std::string get_str_material_domain(MaterialDomain materialDomain);
			static MaterialDomain get_enum_material_domain(const std::string& materialDomain);
			static std::string get_str_material_blend_mode(MaterialBlendMode materialBlendMode);
			static MaterialBlendMode get_enum_material_blend_mode(const std::string& materialBlendMode);
			static std::string get_str_material_shading_model(MaterialShadingModel materialShadingModel);
			static MaterialShadingModel get_enum_material_shading_model(const std::string& materialShadingModel);
			static std::string get_str_material_type(MaterialType materialType);
			static MaterialType get_enum_material_type(const std::string& materialType);
	};          
}