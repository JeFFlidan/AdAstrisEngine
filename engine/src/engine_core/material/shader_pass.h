#pragma once

#include "material_common.h"

namespace ad_astris::ecore::material
{
	class ShaderPass
	{
		public:
			ShaderPass() = default;
			ShaderPass(ShaderPassType shaderPassType);
			// Need this constructor to create new shader pass for GeneralMaterialTemplate
			ShaderPass(ShaderUUIDContext& shaderUUIDContext, ShaderPassType shaderPassType);

			ShaderHandleContext& get_shader_handle_context()
			{
				return _shaderPassInfo.shaderHandleContext;
			}

			ShaderUUIDContext& get_shader_uuid_context()
			{
				return _shaderPassInfo.shaderUUIDContext;
			}

			std::string serialize();
			void deserialize(std::string& metadata);

			ShaderPassType get_type()
			{
				return _shaderPassInfo.type;
			}
		
		private:
			ShaderPassInfo _shaderPassInfo;
	};
}