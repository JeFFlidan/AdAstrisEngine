#pragma once

#include "material_common.h"
#include "shader_pass.h"
#include "engine_core/object.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecore
{
	class GeneralMaterialTemplate : public Object
	{
		public:
			GeneralMaterialTemplate() = default;
			// Need this constructor to create new material template.
			GeneralMaterialTemplate(material::GeneralMaterialTemplateInfo& templateInfo, ObjectName* objectName);

			std::unordered_map<material::ShaderPassType, material::ShaderPass>& get_shader_passes()
			{
				return _templateInfo.shaderPassByItsType;
			}

			material::ShaderPass get_shader_pass(material::ShaderPassType shaderPassType)
			{
				auto it = _templateInfo.shaderPassByItsType.find(shaderPassType);
				if (it == _templateInfo.shaderPassByItsType.end())
					LOG_FATAL("GeneralMaterialTemplate::get_shader_pass(): Material template {} doesn't have shader pass {}", _name->get_full_name(), material::Utils::get_str_shader_pass_type(shaderPassType))
				return it->second;
			}

		private:
			material::GeneralMaterialTemplateInfo _templateInfo;
		
		public:
			// ========== Begin Object interface ==========
				
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* templateName) override;
		
			virtual bool is_resource() override
			{
				return false;				
			}
		
			virtual UUID get_uuid() override
			{
				return _templateInfo.uuid;
			}
		
			virtual std::string get_description() override
			{
				// TODO
				return std::string();
			}
		
			virtual std::string get_type() override
			{
				return "material_template";
			}

			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;

			// ========== End Object interface ==========
	};
}

namespace ad_astris::resource
{
	template<>
	struct FirstCreationContext<ecore::GeneralMaterialTemplate>
	{
		std::vector<ecore::material::ShaderPassCreateInfo> shaderPassCreateInfos;
		std::string materialTemplateName;
	};
}
