#pragma once

#include "engine_core/object.h"
#include <json/json.hpp>

namespace ad_astris::ecore
{
	class IMaterialParams
	{
		public:
			virtual ~IMaterialParams() = default;
			virtual nlohmann::json serialize() = 0;
			virtual void deserialize(const nlohmann::json& json) = 0;
			virtual void set_shader_data(void* shaderData) = 0;
			virtual void draw_ui() = 0;
	};

	struct MaterialTemplateCreateInfo
	{
		std::string name;
	};

	struct MaterialTemplateInfo
	{
		
	};

	class MaterialTemplate
	{
		public:
			IMaterialParams* create_material_params() const;
			void destroy_material_params(IMaterialParams* materialParams) const;

		private:
			MaterialTemplateInfo _info;
			UUID _uuid;
	};
}