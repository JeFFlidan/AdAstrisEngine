#include "material_manager.h"
#include "render_pass_context.h"
#include "resource_manager/resource_events.h"
#include "core/timer.h"

using namespace ad_astris::renderer::impl;

MaterialManager::MaterialManager(MaterialManagerInitializationContext& initializationContext)
	: _shaderCompiler(initializationContext.shaderCompiler), _eventManager(initializationContext.eventManager),
		_taskComposer(initializationContext.taskComposer), _resourceManager(initializationContext.resourceManager),
		_rhi(initializationContext.rhi)
{
	subscribe_to_events();
}

void MaterialManager::create_material_templates(RenderPassContext* renderPassContext)
{
	tasks::TaskGroup* shaderTaskGroup = _taskComposer->allocate_task_group();
	tasks::TaskGroup* materialTemplateTaskGroup = _taskComposer->allocate_task_group();

	Timer timer;
	
	for (auto shader : _loadedShaders)
	{
		_taskComposer->execute(*shaderTaskGroup, [shader, this](tasks::TaskExecutionInfo execInfo)
		{
			ecore::shader::CompilationContext compilationContext = shader->get_compilation_context();
			_shaderCompiler->compile_shader_into_spv(compilationContext);
			shader->set_shader_compiled_flag();

			rhi::Shader apiShader;
			_rhi->create_shader(&apiShader, &shader->get_shader_info());
			std::scoped_lock<std::mutex> lock(_mutex);
			_apiShaderHandleByUUID[shader->get_uuid()] = apiShader;
		});
	}

	_taskComposer->wait(*shaderTaskGroup);
	LOG_INFO("MaterialManager::create_material_templates(): Shaders compilation time: {} ms", timer.elapsed_milliseconds())
	LOG_INFO("API SHADER COUNT: {}", _apiShaderHandleByUUID.size())

	for (auto materialTemplateHandle : _loadedMaterialTemplates)
	{
		_taskComposer->execute(*materialTemplateTaskGroup, [this, materialTemplateHandle](tasks::TaskExecutionInfo execInfo)
		{
			//create_material_template(materialTemplateHandle);
		});
	}

	_taskComposer->wait(*materialTemplateTaskGroup);
	_loadedShaders.clear();
	_loadedMaterialTemplates.clear();
}

void MaterialManager::create_materials()
{
	
}

void MaterialManager::subscribe_to_events()
{
	events::EventDelegate<resource::ShaderLoadedEvent> shaderLoadedDelegate = [&](resource::ShaderLoadedEvent& event)
	{
		_loadedShaders.insert(event.get_shader_handle().get_resource());
	};

	events::EventDelegate<resource::MaterialTemplateLoadedEvent> materialTemplateLoadedDelegate = [&](resource::MaterialTemplateLoadedEvent& event)
	{
		_loadedMaterialTemplates.insert(event.get_material_template_handle().get_resource());
	};

	events::EventDelegate<resource::MaterialTemplateCreatedEvent> materialTemplateCreatedDelegate = [&](resource::MaterialTemplateCreatedEvent& event)
	{
		// TODO Improve this event
		_loadedMaterialTemplates.insert(event.get_material_template_handle().get_resource());
	};

	_eventManager->subscribe(shaderLoadedDelegate);
	_eventManager->subscribe(materialTemplateLoadedDelegate);
	_eventManager->subscribe(materialTemplateCreatedDelegate);
}

void MaterialManager::create_material_template(ecore::MaterialTemplateHandle materialTemplateHandle, RenderPassContext* renderPassContext)
{
	ecore::MaterialTemplate* materialTemplate = materialTemplateHandle.get_resource();
	auto& shaderPasses = materialTemplate->get_shader_passes();
	RendererMaterialTemplate rendererMaterialTemplate(materialTemplateHandle);
	
	for (auto& shaderPassInfo : shaderPasses)
	{
		ecore::material::ShaderPass& shaderPass = shaderPassInfo.second;
		ecore::material::ShaderHandleContext shaderHandleContext = shaderPass.get_shader_handle_context();
		std::vector<ecore::ShaderHandle> shaderHandles;
		shaderHandleContext.get_all_valid_shader_handles(shaderHandles);

		rhi::RenderPass* renderPass = renderPassContext->get_render_pass(shaderPass.get_type());
		rhi::Pipeline pipeline;
		
		if (materialTemplate->get_material_type() == ecore::material::MaterialType::GRAPHICS)
		{
			rhi::GraphicsPipelineInfo pipelineInfo;
			
			switch (shaderPass.get_type())
			{
				case ecore::material::ShaderPassType::GBUFFER:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 4);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::GEOMETRY);
					break;
				}
				case ecore::material::ShaderPassType::DEFERRED_LIGHTING:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, false, false, false, ecore::model::VertexFormat::F32_PC);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::LIGHTING);
					break;
				}
				case ecore::material::ShaderPassType::OIT_PREPASS:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::GEOMETRY);
					break;
				}
				case ecore::material::ShaderPassType::OIT:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 2, false);
					break;
				}
				case ecore::material::ShaderPassType::TAA:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, false, false, false, ecore::model::VertexFormat::F32_PC);
					break;
				}
				case ecore::material::ShaderPassType::COMPOSITE:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 2, false, false, false, ecore::model::VertexFormat::F32_PC);
					break;
				}
				case ecore::material::ShaderPassType::POSTPROCESSING:
				{
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, false, false, false, ecore::model::VertexFormat::F32_PC);
					break;
				}
				case ecore::material::ShaderPassType::DIRECTIONAL_LIGHT_SHADOWS:
				{
					// TODO I must make pipelines for every directional light, not only for one
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, true, true, true);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::GEOMETRY);
					break;
				}
				case ecore::material::ShaderPassType::POINT_LIGHT_SHADOWS:
				{
					// TODO I must make pipelines for every point light, not only for one
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, true, true, true);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::GEOMETRY);
					break;
				}
				case ecore::material::ShaderPassType::SPOT_LIGHT_SHADOWS:
				{
					// TODO I must make pipelines for every spot light, not only for one
					setup_default_graphics_pipeline_info(pipelineInfo, renderPass, shaderHandles, 1, true, true, true);
					rendererMaterialTemplate.add_requirement(RendererMaterialTemplateRequirements::GEOMETRY);
					break;
				}
			}
			
			_rhi->create_graphics_pipeline(&pipeline, &pipelineInfo);
		}
		else if (materialTemplate->get_material_type() == ecore::material::MaterialType::COMPUTE)
		{
			rhi::ComputePipelineInfo pipelineInfo;
			pipelineInfo.shaderStage = _apiShaderHandleByUUID[shaderHandles[0].get_resource()->get_uuid()];
			_rhi->create_compute_pipeline(&pipeline, &pipelineInfo);
		}

		RendererShaderPass rendererShaderPass(pipeline, shaderPass);
		rendererMaterialTemplate.add_shader_pass(rendererShaderPass);
	}

	std::scoped_lock<std::mutex> lock(_mutex);
	uint32_t index = _rendererMaterialTemplates.size();
	_rendererMaterialTemplates.push_back(rendererMaterialTemplate);
	_rendererMaterialTemplateIndexByUUID[materialTemplate->get_uuid()] = index;
}

void MaterialManager::setup_default_graphics_pipeline_info(
	rhi::GraphicsPipelineInfo& pipelineInfo,
	rhi::RenderPass* renderPass,
	std::vector<ecore::ShaderHandle>& shaderHandles,
	uint32_t attachmentCount,
	bool isCullingEnabled,
	bool isDepthTestEnabled,
	bool isBiasEnabled,
	ecore::model::VertexFormat vertexFormat)
{
	pipelineInfo.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;

	switch (vertexFormat)
	{
		case ecore::model::VertexFormat::F32_PNTC:
		{
			ecore::model::Utils::setup_f32_pntc_format_description(pipelineInfo.bindingDescriptrions, pipelineInfo.attributeDescriptions);
			break;
		}
		case ecore::model::VertexFormat::F32_PC:
		{
			ecore::model::Utils::setup_f32_pc_format_description(pipelineInfo.bindingDescriptrions, pipelineInfo.attributeDescriptions);
		}
	}
	
	pipelineInfo.multisampleState.isEnabled = false;
	pipelineInfo.multisampleState.sampleCount = rhi::SampleCount::BIT_1;
	
	pipelineInfo.rasterizationState.cullMode = isCullingEnabled ? rhi::CullMode::BACK : rhi::CullMode::NONE;
	pipelineInfo.rasterizationState.frontFace = rhi::FrontFace::COUNTER_CLOCKWISE;
	pipelineInfo.rasterizationState.lineWidth = 1.0f;
	pipelineInfo.rasterizationState.polygonMode = rhi::PolygonMode::FILL;
	pipelineInfo.rasterizationState.isBiasEnabled = isBiasEnabled;

	for (auto shaderHandle : shaderHandles)
	{
		pipelineInfo.shaderStages.push_back(_apiShaderHandleByUUID[shaderHandle.get_resource()->get_uuid()]);
	}

	pipelineInfo.renderPass = *renderPass;
	
	pipelineInfo.colorBlendState.logicOp = rhi::LogicOp::COPY;
	pipelineInfo.colorBlendState.isLogicOpEnabled = false;
	for (int32_t i = 0; i != attachmentCount; ++i)
	{
		rhi::ColorBlendAttachmentState attachState;
		attachState.isBlendEnabled = false;
		pipelineInfo.colorBlendState.colorBlendAttachments.push_back(attachState);
	}

	pipelineInfo.depthStencilState.isStencilTestEnabled = false;
	pipelineInfo.depthStencilState.isDepthTestEnabled = isDepthTestEnabled;
	pipelineInfo.depthStencilState.isDepthWriteEnabled = isDepthTestEnabled;
	pipelineInfo.depthStencilState.compareOp = rhi::CompareOp::LESS_OR_EQUAL;
}

RendererMaterialTemplate::RendererMaterialTemplate(ecore::MaterialTemplateHandle materialTemplateHandle)
	: _materialTemplateHandle(materialTemplateHandle)
{
	
}

void RendererMaterialTemplate::add_shader_pass(RendererShaderPass& shaderPass)
{
	auto it = _shaderPassByItsType.find(shaderPass.get_type());
	if (it != _shaderPassByItsType.end())
	{
		std::string templateName = _materialTemplateHandle.get_resource()->get_name()->get_full_name();
		std::string shaderPassStrType = ecore::material::Utils::get_str_shader_pass_type(shaderPass.get_type());
		LOG_ERROR("RendererMaterialTemplate::add_shader_pass(): Material template {} has shader pass {}", templateName, shaderPassStrType)
		return;
	}

	//_shaderPassByItsType[shaderPass.get_type()] = shaderPass;
	_shaderPassByItsType.insert({ shaderPass.get_type(), shaderPass } );
}
		
RendererShaderPass* RendererMaterialTemplate::get_shader_pass(ecore::material::ShaderPassType shaderPassType)
{
	auto it = _shaderPassByItsType.find(shaderPassType);
	if (it == _shaderPassByItsType.end())
	{
		std::string templateName = _materialTemplateHandle.get_resource()->get_name()->get_full_name();
		std::string shaderPassStrType = ecore::material::Utils::get_str_shader_pass_type(shaderPassType);
		LOG_FATAL("RendererMaterialTemplate::get_shader_pass(): Material template {} doesn't have shader pass {}", templateName, shaderPassStrType)
	}

	return &it->second;
}

void RendererMaterialTemplate::add_requirement(RendererMaterialTemplateRequirements newRequirement)
{
	_requirements |= newRequirement;
}

RendererMaterialTemplateRequirements RendererMaterialTemplate::get_requirement()
{
	return _requirements;
}
