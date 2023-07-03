#pragma once

#include "fwd.h"
#include "shader.h"
#include "material_common.h"
#include "engine_core/object.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecore
{
	struct ShaderUUIDContext
	{
		std::vector<UUID> shaderUUIDs;
	};
	
	struct ShaderHandleContext
	{
		ShaderHandle vertexShader{ nullptr };
		ShaderHandle fragmentShader{ nullptr };
		ShaderHandle tessControlShader{ nullptr };
		ShaderHandle tessEvaluationShader{ nullptr };
		ShaderHandle geometryShader{ nullptr };
		ShaderHandle computeShader{ nullptr };
		ShaderHandle meshShader{ nullptr };
		ShaderHandle taskShader{ nullptr };
		ShaderHandle rayGenerationShader{ nullptr };
		ShaderHandle rayIntersectionShader{ nullptr };
		ShaderHandle rayAnyHitShader{ nullptr };
		ShaderHandle rayClosestHit{ nullptr };
		ShaderHandle rayMiss{ nullptr };
		ShaderHandle rayCallable{ nullptr };
	};
	
	class GeneralMaterialTemplate : public Object
	{
		public:
			GeneralMaterialTemplate() = default;
			GeneralMaterialTemplate(ShaderUUIDContext& shaderUUIDContext);

			ShaderHandleContext& get_shader_handle_context()
			{
				return _shaderHandleContext;
			}

			ShaderUUIDContext& get_shader_uuid_context()
			{
				return _shaderUUIDContext;
			}

		private:
			ShaderHandleContext _shaderHandleContext;
			ShaderUUIDContext _shaderUUIDContext;
			UUID _uuid;
		
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
				return _uuid;
			}
			virtual std::string get_description() override;
			virtual std::string get_type() override;

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
