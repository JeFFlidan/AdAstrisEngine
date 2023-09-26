#include "object_test.h"
#include "profiler/logger.h"
#include "engine_core/model/model_common.h"

using namespace ad_astris::renderer::impl;

ObjectTest::ObjectTest(RenderingInitContext& initContext, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager)
	: RenderingBase(initContext)
{
	rhi::Shader* vertexShader = shaderManager->load_shader("engine/shaders/objectVS.hlsl", rhi::ShaderType::VERTEX);
	auto fragmentShader = shaderManager->load_shader("engine/shaders/objectPS.hlsl", rhi::ShaderType::FRAGMENT);
	
	rhi::GraphicsPipelineInfo pipelineInfo;
	
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

	pipelineInfo.rasterizationState.cullMode = rhi::CullMode::NONE;
	pipelineInfo.rasterizationState.polygonMode = rhi::PolygonMode::FILL;
	pipelineInfo.rasterizationState.isBiasEnabled = false;
	pipelineInfo.rasterizationState.frontFace = rhi::FrontFace::COUNTER_CLOCKWISE;

	pipelineInfo.shaderStages.push_back(*vertexShader);
	pipelineInfo.shaderStages.push_back(*fragmentShader);

	rhi::ColorBlendAttachmentState attachState;
	attachState.isBlendEnabled = false;
	
	pipelineInfo.colorBlendState.logicOp = rhi::LogicOp::COPY;
	pipelineInfo.colorBlendState.colorBlendAttachments.push_back(attachState);
	pipelineInfo.colorBlendState.isLogicOpEnabled = false;

	pipelineInfo.depthStencilState.isDepthWriteEnabled = false;
	pipelineInfo.depthStencilState.isDepthWriteEnabled = false;
	pipelineInfo.depthStencilState.isStencilTestEnabled = false;
	pipelineInfo.depthStencilState.compareOp = rhi::CompareOp::LESS_OR_EQUAL;

	_rhi->create_graphics_pipeline(&_pipeline, &pipelineInfo);
	LOG_INFO("ObjectTest::ObjectTest(): Created pipeline")
}

void ObjectTest::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("ObjectOutput", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("ObjectOutput", "ObjectOutput");

	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("ObjectTest", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("ObjectOutput");
	renderPass->set_executor(this);
}

void ObjectTest::execute(rhi::CommandBuffer* cmd)
{
	rhi::Viewport viewport;
	viewport.width = _mainWindow->get_width();
	viewport.height = _mainWindow->get_height();
	std::vector<rhi::Viewport> viewports = { viewport };
	_rhi->set_viewports(cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = _mainWindow->get_width();
	scissor.bottom = _mainWindow->get_height();
	std::vector<rhi::Scissor> scissors = { scissor };
	_rhi->set_scissors(cmd, scissors);
	_rhi->bind_pipeline(cmd, &_pipeline);
	rhi::Buffer* vertexBuffer = _sceneManager->get_vertex_buffer_f32pntc();
	rhi::Buffer* indexBuffer = _sceneManager->get_index_buffer_f32pntc();
	_rhi->bind_vertex_buffer(cmd, vertexBuffer);
	_rhi->bind_index_buffer(cmd, indexBuffer);
	_rhi->draw_indexed(cmd, 27018, 1, 0, 0, 0);
}
