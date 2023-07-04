#pragma once

#include "engine_core/fwd.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecore::material
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
        ShaderHandle rayClosestHitShader{ nullptr };
        ShaderHandle rayMissShader{ nullptr };
        ShaderHandle rayCallableShader{ nullptr };

        void get_all_valid_shader_handles(std::vector<ShaderHandle>& shaderHandles);
    };

    struct GeneralMaterialTemplateInfo
    {
        ShaderUUIDContext shaderUUIDContext;
        ShaderHandleContext shaderHandleContext;
        UUID uuid;
    };
    
    class Utils
    {
        public:
            static std::string pack_general_material_template_info(GeneralMaterialTemplateInfo& info);
            static GeneralMaterialTemplateInfo unpack_general_material_template_info(std::string& metadata);
    };          
}