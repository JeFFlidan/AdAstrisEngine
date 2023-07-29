#pragma once

#include "material_common.h"

namespace ad_astris::ecore::material
{
	class ShaderPass
	{
		public:
			ShaderPass() = default;
			ShaderPass(const std::string& shaderPassName);
			// Need this constructor to create new shader pass for GeneralMaterialTemplate
			ShaderPass(ShaderHandleContext& shaderHandleContext, std::string& shaderPassName);

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

			std::string get_name()
			{
				return _name;
			}
		
		private:
			ShaderPassInfo _shaderPassInfo;
			std::string _name;
	};
}