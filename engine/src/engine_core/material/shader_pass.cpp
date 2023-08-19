#include "shader_pass.h"
#include "shader.h"
#include "rhi/resources.h"

using namespace ad_astris::ecore::material;

ShaderPass::ShaderPass(ShaderPassType shaderPassType)
{
	_shaderPassInfo.type = shaderPassType;
}

ShaderPass::ShaderPass(ShaderUUIDContext& shaderUUIDContext, ShaderPassType shaderPassType)
{
	_shaderPassInfo.shaderUUIDContext = shaderUUIDContext;
	_shaderPassInfo.type = shaderPassType;
}

std::string ShaderPass::serialize()
{
	return Utils::pack_shader_pass_info(_shaderPassInfo);
}

void ShaderPass::deserialize(std::string& metadata)
{
	ShaderPassType shaderPassType = _shaderPassInfo.type;
	_shaderPassInfo = Utils::unpack_shader_pass_info(metadata);
	_shaderPassInfo.type = shaderPassType;
}
