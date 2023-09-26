#include "triangle_test.h"
#include "profiler/logger.h"

using namespace ad_astris::renderer::impl;

TriangleTest::TriangleTest(RenderingInitContext& initContext, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager)
	: RenderingBase(initContext)
{
	rhi::Shader* vertexShader = shaderManager->load_shader("engine/shaders/triangleVS.hlsl", rhi::ShaderType::VERTEX);
	auto fragmentShader = shaderManager->load_shader("engine/shaders/trianglePS.hlsl", rhi::ShaderType::FRAGMENT);
	
	rhi::GraphicsPipelineInfo pipelineInfo;
	
	pipelineInfo.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;

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
	LOG_INFO("TriangleTest::TriangleTest(): Created pipeline")
}

void TriangleTest::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("TriangleOutput", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("TriangleOutput", "TriangleOutput");

	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("TriangleTest", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("TriangleOutput");
	renderPass->set_executor(this);
}

void TriangleTest::execute(rhi::CommandBuffer* cmd)
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
	_rhi->draw(cmd, 3);
}
