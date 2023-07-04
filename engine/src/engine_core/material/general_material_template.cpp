#include "general_material_template.h"
#include "general_material_template.h"
#include "rhi/resources.h"
#include "shader.h"

using namespace ad_astris;
using namespace ecore;

GeneralMaterialTemplate::GeneralMaterialTemplate(material::ShaderUUIDContext& shaderUUIDContext, const std::string& templateName)
{
	_templateInfo.shaderUUIDContext = shaderUUIDContext;
	_name = ObjectName(templateName.c_str());
}

void GeneralMaterialTemplate::serialize(io::IFile* file)
{
	std::string newMetadata = material::Utils::pack_general_material_template_info(_templateInfo);
	file->set_metadata(newMetadata);
}

void GeneralMaterialTemplate::deserialize(io::IFile* file, ObjectName* newName)
{
	_templateInfo = material::Utils::unpack_general_material_template_info(file->get_metadata());
	if (!newName)
	{
		_name = ObjectName(file->get_file_name().c_str());
	}
	else
	{
		_name = *newName;
	}
	_path = file->get_file_path();
}

void GeneralMaterialTemplate::load_required_shaders(resource::ResourceManager* resourceManager)
{
	for (auto& shaderUUID : _templateInfo.shaderUUIDContext.shaderUUIDs)
	{
		ShaderHandle shaderHandle = resourceManager->get_resource<Shader>(shaderUUID);
		Shader* shaderObject = shaderHandle.get_resource();
		if (shaderObject->get_shader_type() == rhi::ShaderType::UNDEFINED)
		{
			LOG_ERROR("GeneralMaterialTemplate::load_required_shaders(): Shader {} has undefined type.", shaderObject->get_path().c_str())
			continue;
		}

		switch (shaderObject->get_shader_type())
		{
			case rhi::ShaderType::VERTEX:
				_templateInfo.shaderHandleContext.vertexShader = shaderHandle;
				break;
			case rhi::ShaderType::FRAGMENT:
				_templateInfo.shaderHandleContext.fragmentShader = shaderHandle;
				break;
			case rhi::ShaderType::TESSELLATION_CONTROL:
				_templateInfo.shaderHandleContext.tessControlShader = shaderHandle;
				break;
			case rhi::ShaderType::TESSELLATION_EVALUATION:
				_templateInfo.shaderHandleContext.tessEvaluationShader = shaderHandle;
				break;
			case rhi::ShaderType::GEOMETRY:
				_templateInfo.shaderHandleContext.geometryShader = shaderHandle;
				break;
			case rhi::ShaderType::COMPUTE:
				_templateInfo.shaderHandleContext.computeShader = shaderHandle;
				break;
			case rhi::ShaderType::MESH:
				_templateInfo.shaderHandleContext.meshShader = shaderHandle;
				break;
			case rhi::ShaderType::TASK:
				_templateInfo.shaderHandleContext.taskShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_GENERATION:
				_templateInfo.shaderHandleContext.rayGenerationShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_INTERSECTION:
				_templateInfo.shaderHandleContext.rayIntersectionShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_ANY_HIT:
				_templateInfo.shaderHandleContext.rayAnyHitShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_CLOSEST_HIT:
				_templateInfo.shaderHandleContext.rayClosestHitShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_MISS:
				_templateInfo.shaderHandleContext.rayMissShader = shaderHandle;
				break;
			case rhi::ShaderType::RAY_CALLABLE:
				_templateInfo.shaderHandleContext.rayClosestHitShader = shaderHandle;
				break;
		}
	}
}
