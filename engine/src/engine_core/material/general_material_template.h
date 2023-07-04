#pragma once

#include "material_common.h"
#include "engine_core/object.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecore
{
	class GeneralMaterialTemplate : public Object
	{
		public:
			GeneralMaterialTemplate() = default;
			// Need this constructor to create new material template.
			GeneralMaterialTemplate(material::ShaderUUIDContext& shaderUUIDContext, const std::string& templateName);

			material::ShaderHandleContext& get_shader_handle_context()
			{
				return _templateInfo.shaderHandleContext;
			}

			material::ShaderUUIDContext& get_shader_uuid_context()
			{
				return _templateInfo.shaderUUIDContext;
			}

			void load_required_shaders(resource::ResourceManager* resourceManager);

		private:
			material::GeneralMaterialTemplateInfo _templateInfo;
		
		public:
			// ========== Begin Object interface ==========
				
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) override;
		
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

			// ========== End Object interface ==========
	};
}

namespace ad_astris::resource
{
	template<>
	struct FirstCreationContext<ecore::GeneralMaterialTemplate>
	{
		io::URI vertexShaderPath;
		io::URI fragmentShaderPath;
		io::URI tessControlShader;
		io::URI tessEvaluationShader;
		io::URI geometryShader;
		io::URI computeShader;
		io::URI meshShader;
		io::URI taskShader;
		io::URI rayGenerationShader;
		io::URI rayIntersectionShader;
		io::URI rayAnyHitShader;
		io::URI rayClosestHit;
		io::URI rayMiss;
		io::URI rayCallable;
		std::string materialTemplateName;
	};
}
