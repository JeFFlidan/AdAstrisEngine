#include "shader_pass.h"
#include "shader.h"
#include "rhi/resources.h"

using namespace ad_astris::ecore::material;

ShaderPass::ShaderPass(const std::string& shaderPassName) : _name(shaderPassName)
{
	
}

ShaderPass::ShaderPass(ShaderHandleContext& shaderHandleContext, std::string& shaderPassName) : _name(shaderPassName)
{
	_shaderPassInfo.shaderHandleContext = shaderHandleContext;
}

std::string ShaderPass::serialize()
{
	return Utils::pack_shader_pass_info(_shaderPassInfo);
}

void ShaderPass::deserialize(std::string& metadata)
{
	_shaderPassInfo = Utils::unpack_shader_pass_info(metadata);
}
