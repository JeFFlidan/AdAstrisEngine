#pragma once

#include "engine_core/object.h"
#include "material_parameter_struct.h"
#include "material_template_types.h"
#include "material_parameter_struct_metadata.h"

namespace ad_astris::ecore
{
	struct MaterialTemplateCreateInfo
	{
		io::URI resourceFolderPath;
		io::URI materialDataFolderPath;
		std::string name;
	};

	struct MaterialTemplateUpdateInfo
	{
		MaterialDomain domain{ MaterialDomain::UNDEFINED };
		MaterialBlendMode blendMode{ MaterialBlendMode::UNDEFINED };
		MaterialShadingModel shadingModel{ MaterialShadingModel::UNDEFINED };
		io::URI materialDataFolderPath;
	};

	struct MaterialTemplateInfo
	{
		std::unique_ptr<MaterialParameterStructMetadata> parameterStructMetadata{ nullptr };
		MaterialDomain domain{ MaterialDomain::SURFACE };
		MaterialBlendMode blendMode{ MaterialBlendMode::OPAQUE_ };
		MaterialShadingModel shadingModel{ MaterialShadingModel::DEFAULT_LIT };
		std::unordered_map<rhi::ShaderType, io::URI> shaderPathByType; 
	};
	
	class MaterialTemplate : public Object
	{
		public:
			MaterialTemplate() = default;
			MaterialTemplate(const MaterialTemplateCreateInfo& createInfo, ObjectName* name);
		
			MaterialParameterStruct* create_parameter_struct() const;
			void update(const MaterialTemplateUpdateInfo& updateInfo);
		
			const MaterialTemplateInfo& get_info() const { return _info; }

			// ========== Begin Object interface ==========
		
			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Material template"; }
			std::string get_type() override { return "material_template"; }

			// ========== End Object interface ==========

		private:
			MaterialTemplateInfo _info;
			UUID _uuid;

			void read_material_metadata(const io::URI& materialFolderPath, nlohmann::json& outMetadata) const;
			void get_shader_paths(const nlohmann::json& metadata);
	};
}