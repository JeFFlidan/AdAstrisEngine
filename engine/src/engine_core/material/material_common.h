#pragma once

#include "engine_core/fwd.h"
#include "resource_manager/resource_manager.h"
#include <unordered_map>

namespace ad_astris::ecore::material
{
    class ShaderPass;
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

    struct ShaderPassInfo
    {
        ShaderUUIDContext shaderUUIDContext;
        ShaderHandleContext shaderHandleContext;
    };

    struct ShaderPassCreateInfo
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
        std::string passName;
    };

    struct GeneralMaterialTemplateInfo
    {
        std::unordered_map<std::string, ShaderPass> shaderPassByItsName;
        std::vector<std::string> shaderPassesOrder;
        UUID uuid;
    };

    struct MaterialInfo
    {
        
    };
    
    class Utils
    {
        public:
            static std::string pack_general_material_template_info(GeneralMaterialTemplateInfo& info);
            static GeneralMaterialTemplateInfo unpack_general_material_template_info(std::string& metadata);
            static std::string pack_shader_pass_info(ShaderPassInfo& info);
            static ShaderPassInfo unpack_shader_pass_info(std::string& metadata);
    };          
}