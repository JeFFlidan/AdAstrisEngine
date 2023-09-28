#include "pipeline_manager.h"
#include "engine_core/model/model_common.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

PipelineManager::PipelineManager(IRendererResourceManager* rendererResourceManager)
	: _rendererResourceManager(rendererResourceManager)
{
	
}

void PipelineManager::init(PipelineManagerInitContext& initContext)
{
	assert(initContext.rhi);
	assert(initContext.shaderManager);
	assert(initContext.taskComposer);

	_rhi = initContext.rhi;
	_taskComposer = initContext.taskComposer;
	_shaderManager = initContext.shaderManager;

	load_builtin_shaders();
}

void PipelineManager::bind_render_pass_to_pipeline(IRenderPass* renderPass, BuiltinPipelineType type)
{
	_builtinPipelineStateByItsType[type].renderPass = static_cast<RenderPass*>(renderPass);
}

void PipelineManager::create_builtin_pipelines()
{
	tasks::TaskGroup taskGroup;
	_taskComposer->execute(taskGroup, [this](tasks::TaskExecutionInfo){ create_gbuffer_pipeline(); });
	_taskComposer->execute(taskGroup, [this](tasks::TaskExecutionInfo){ create_deferred_lighting_pipeline(); });
	_taskComposer->wait(taskGroup);
}

rhi::Pipeline* PipelineManager::get_builtin_pipeline(BuiltinPipelineType pipelineType)
{
	auto it = _builtinPipelineStateByItsType.find(pipelineType);
	if (it == _builtinPipelineStateByItsType.end())
		LOG_FATAL("PipelineManager::get_builtin_pipeine(): No builtin pipeline")
	return &it->second.pipeline;
}

void PipelineManager::load_builtin_shaders()
{
	tasks::TaskGroup taskGroup;
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo execInfo) { _shaderManager->load_shader("engine/shaders/objectVS.hlsl", rhi::ShaderType::VERTEX); });
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo execInfo) { _shaderManager->load_shader("engine/shaders/gbufferPS.hlsl", rhi::ShaderType::FRAGMENT); });
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo execInfo) { _shaderManager->load_shader("engine/shaders/outputPlaneVS.hlsl", rhi::ShaderType::VERTEX); });
	_taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo execInfo) { _shaderManager->load_shader("engine/shaders/deferredLightingPS.hlsl", rhi::ShaderType::FRAGMENT); });
	_taskComposer->wait(taskGroup);
}

void PipelineManager::setup_default_graphics_pipeline_info(rhi::GraphicsPipelineInfo& pipelineInfo)
{	
	pipelineInfo.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;

	rhi::VertexAttributeDescription vertexAttributeDescription;
	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.format = rhi::Format::R32G32B32_SFLOAT;
	vertexAttributeDescription.location = 0;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PNTC, position);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.location = 1;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PNTC, normal);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.location = 2;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PNTC, tangent);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.format = rhi::Format::R32G32_SFLOAT;
	vertexAttributeDescription.location = 3;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PNTC, texCoord);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	rhi::VertexBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(ecore::model::VertexF32PNTC);
	pipelineInfo.bindingDescriptrions.push_back(bindingDescription);

	pipelineInfo.multisampleState.isEnabled = false;
	pipelineInfo.multisampleState.sampleCount = rhi::SampleCount::BIT_1;

	pipelineInfo.rasterizationState.cullMode = rhi::CullMode::BACK;
	pipelineInfo.rasterizationState.polygonMode = rhi::PolygonMode::FILL;
	pipelineInfo.rasterizationState.isBiasEnabled = false;
	pipelineInfo.rasterizationState.frontFace = rhi::FrontFace::CLOCKWISE;

	rhi::ColorBlendAttachmentState attachState;
	attachState.isBlendEnabled = false;
	
	pipelineInfo.colorBlendState.logicOp = rhi::LogicOp::COPY;
	pipelineInfo.colorBlendState.colorBlendAttachments.push_back(attachState);
	pipelineInfo.colorBlendState.isLogicOpEnabled = false;

	pipelineInfo.depthStencilState.isDepthTestEnabled = false;
	pipelineInfo.depthStencilState.isDepthWriteEnabled = false;
	pipelineInfo.depthStencilState.isStencilTestEnabled = false;
	pipelineInfo.depthStencilState.compareOp = rhi::CompareOp::LESS_OR_EQUAL;
}

void PipelineManager::setup_formats(BuiltinPipelineType type, std::vector<rhi::Format>& colorAttachFormats, rhi::Format& depthAttachFormat)
{
	auto it = _builtinPipelineStateByItsType.find(type);
	if (it == _builtinPipelineStateByItsType.end())
		LOG_FATAL("PipelineManager::get_formats(): No pipeline state")

	std::vector<TextureDesc*> colorOutputs = it->second.renderPass->get_color_outputs();
	for (auto& colorOutput : colorOutputs)
	{
		auto textureView = _rendererResourceManager->get_texture_view(colorOutput->get_name());
		colorAttachFormats.push_back(textureView->texture->textureInfo.format);
	}

	TextureDesc* depthOutput = it->second.renderPass->get_depth_stencil_output();
	if (depthOutput)
	{
		auto textureView = _rendererResourceManager->get_texture_view(depthOutput->get_name());
		depthAttachFormat = textureView->texture->textureInfo.format;
	}
}

void PipelineManager::create_gbuffer_pipeline()
{
	rhi::GraphicsPipelineInfo pipelineInfo;
	setup_default_graphics_pipeline_info(pipelineInfo);
	pipelineInfo.depthStencilState.isDepthTestEnabled = true;
	pipelineInfo.depthStencilState.isDepthWriteEnabled = true;
	setup_formats(BuiltinPipelineType::GBUFFER, pipelineInfo.colorAttachmentFormats, pipelineInfo.depthFormat);
	rhi::Shader* vertexShader = _shaderManager->get_shader("engine/shaders/objectVS.hlsl");
	rhi::Shader* fragmentShader = _shaderManager->get_shader("engine/shaders/gbufferPS.hlsl");

	pipelineInfo.colorBlendState.colorBlendAttachments.clear();
	for (uint32_t i = 0; i != 3; ++i)
	{
		rhi::ColorBlendAttachmentState attachState;
		attachState.isBlendEnabled = false;
	
		pipelineInfo.colorBlendState.logicOp = rhi::LogicOp::COPY;
		pipelineInfo.colorBlendState.colorBlendAttachments.push_back(attachState);
		pipelineInfo.colorBlendState.isLogicOpEnabled = false;
	}

	pipelineInfo.shaderStages.push_back(*vertexShader);
	pipelineInfo.shaderStages.push_back(*fragmentShader);
	
	rhi::Pipeline pipeline;
	_rhi->create_graphics_pipeline(&pipeline, &pipelineInfo);
	std::scoped_lock<std::mutex> locker(_builtinPipelinesMutex);
	_builtinPipelineStateByItsType[BuiltinPipelineType::GBUFFER].pipeline = pipeline;
}

void PipelineManager::create_deferred_lighting_pipeline()
{
	rhi::GraphicsPipelineInfo pipelineInfo;
	setup_default_graphics_pipeline_info(pipelineInfo);
	setup_formats(BuiltinPipelineType::DEFERRED_LIGHTING, pipelineInfo.colorAttachmentFormats, pipelineInfo.depthFormat);
	rhi::Shader* vertexShader = _shaderManager->get_shader("engine/shaders/outputPlaneVS.hlsl");
	rhi::Shader* fragmentShader = _shaderManager->get_shader("engine/shaders/deferredLightingPS.hlsl");

	LOG_WARNING("IS UNDEFINED: {}", pipelineInfo.depthFormat == rhi::Format::UNDEFINED)

	pipelineInfo.shaderStages.push_back(*vertexShader);
	pipelineInfo.shaderStages.push_back(*fragmentShader);

	pipelineInfo.attributeDescriptions.clear();

	rhi::VertexAttributeDescription vertexAttributeDescription;
	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.format = rhi::Format::R32G32B32_SFLOAT;
	vertexAttributeDescription.location = 0;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PC, position);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.format = rhi::Format::R32G32_SFLOAT;
	vertexAttributeDescription.location = 1;
	vertexAttributeDescription.offset = offsetof(ecore::model::VertexF32PC, texCoord);
	pipelineInfo.attributeDescriptions.push_back(vertexAttributeDescription);

	pipelineInfo.bindingDescriptrions.back().stride = sizeof(ecore::model::VertexF32PC);
	
	pipelineInfo.rasterizationState.cullMode = rhi::CullMode::NONE;
	
	rhi::Pipeline pipeline;
	_rhi->create_graphics_pipeline(&pipeline, &pipelineInfo);
	std::scoped_lock<std::mutex> locker(_builtinPipelinesMutex);
	_builtinPipelineStateByItsType[BuiltinPipelineType::DEFERRED_LIGHTING].pipeline = pipeline;

}
