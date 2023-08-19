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
    
    enum class ShaderPassType : uint64_t
    {
        GBUFFER = 0,
        DEFERRED_LIGHTING,
        OIT_PREPASS,
        OIT,        // Order independent transparency
        TAA,
        DIRECTIONAL_LIGHT_SHADOWS,
        POINT_LIGHT_SHADOWS,
        SPOT_LIGHT_SHADOWS,
        COMPOSITE,
        POSTPROCESSING,
        CULLING,
        REDUCE_DEPTH
    };

    struct ShaderPassInfo
    {
        ShaderUUIDContext shaderUUIDContext;
        ShaderHandleContext shaderHandleContext;
        ShaderPassType type;
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
        ShaderPassType passType;
    };

    struct GeneralMaterialTemplateInfo
    {
        std::unordered_map<ShaderPassType, ShaderPass> shaderPassByItsType;
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
            static std::string get_str_shader_pass_type(ShaderPassType shaderPassType);
            static ShaderPassType get_enum_shader_pass_type(const std::string& shaderPassName);
    };          
}