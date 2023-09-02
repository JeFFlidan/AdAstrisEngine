#pragma once

#include "rhi/engine_rhi.h"
#include "engine_core/material/material_template.h"
#include "engine_core/model/model_common.h"
#include "engine_core/material/materials.h"
#include "engine_core/material/shader.h"
#include "engine/render_core_module.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "engine_core/fwd.h"
#include <unordered_set>
#include <unordered_map>

namespace ad_astris::renderer::impl
{
	class RenderPassContext;
	
	class RendererShaderPass
	{
		public:
			RendererShaderPass(rhi::Pipeline& pipeline, ecore::material::ShaderPass& shaderPass)
				: _pipeline(pipeline), _shaderPassType(shaderPass.get_type())
			{
				
			}

			void bind_pipeline(rhi::IEngineRHI* rhi, rhi::CommandBuffer& cmdBuffer)
			{
				rhi->bind_pipeline(&cmdBuffer, &_pipeline);
			}

			ecore::material::ShaderPassType get_type()
			{
				return _shaderPassType;
			}
		
		private:
			rhi::Pipeline _pipeline;
			ecore::material::ShaderPassType _shaderPassType;
	};

	enum class RendererMaterialTemplateRequirements
	{
		UNDEFINED = 0,
		// Material template requires geometry (scene manager will calculate new indirect buffer for material template with this requirement)
		GEOMETRY = 1 << 0,
		// Material template are affected by light sources
		LIGHTING = 1 << 1,
	};

	class RendererMaterialTemplate
	{
		public:
			RendererMaterialTemplate(ecore::MaterialTemplateHandle materialTemplateHandle);

			void add_shader_pass(RendererShaderPass& shaderPass);
			RendererShaderPass* get_shader_pass(ecore::material::ShaderPassType shaderPassType);
			void add_requirement(RendererMaterialTemplateRequirements newRequirement);
			RendererMaterialTemplateRequirements get_requirement();
		
		private:
			std::unordered_map<ecore::material::ShaderPassType, RendererShaderPass> _shaderPassByItsType;
			ecore::MaterialTemplateHandle _materialTemplateHandle;
			RendererMaterialTemplateRequirements _requirements{ RendererMaterialTemplateRequirements::UNDEFINED };
	};
	
	struct MaterialManagerInitializationContext
	{
		rcore::IShaderCompiler* shaderCompiler;
		events::EventManager* eventManager;
		tasks::TaskComposer* taskComposer;
		resource::ResourceManager* resourceManager;
		rhi::IEngineRHI* rhi;
	};

	struct RendererOpaquePBRMaterial
	{
		
	};

	struct RendererTransparentMaterial
	{
		
	};
	
	class MaterialManager
	{
		public:
			MaterialManager(MaterialManagerInitializationContext& initializationContext);

			void create_material_templates(RenderPassContext* renderPassContext);
			void create_materials();
			RendererMaterialTemplate* get_material_template(UUID materialTemplateUUID);

		private:
			rcore::IShaderCompiler* _shaderCompiler;
			events::EventManager* _eventManager;
			tasks::TaskComposer* _taskComposer;
			resource::ResourceManager* _resourceManager;
			rhi::IEngineRHI* _rhi;
			std::unordered_set<ecore::Shader*> _loadedShaders;
			std::unordered_set<ecore::MaterialTemplate*> _loadedMaterialTemplates;
			std::unordered_map<UUID, rhi::Shader> _apiShaderHandleByUUID;
			std::vector<RendererMaterialTemplate> _rendererMaterialTemplates;
			std::unordered_map<UUID, uint32_t> _rendererMaterialTemplateIndexByUUID;

			std::mutex _mutex;

			void subscribe_to_events();
			void create_material_template(ecore::MaterialTemplateHandle materialTemplateHandle, RenderPassContext* renderPassContext);
			void setup_default_graphics_pipeline_info(
				rhi::GraphicsPipelineInfo& pipelineInfo,
				rhi::RenderPass* renderPass,
				std::vector<ecore::ShaderHandle>& shaderHandles,
				uint32_t attachmentCount,
				bool isCullingEnabled = true,
				bool isDepthTestEnabled = true,
				bool isBiasEnabled = false,
				ecore::model::VertexFormat vertexFormat = ecore::model::VertexFormat::F32_PNTC);
	};
}

template<>
struct EnableBitMaskOperator<ad_astris::renderer::impl::RendererMaterialTemplateRequirements>
{
	static const bool enable = true;
};
