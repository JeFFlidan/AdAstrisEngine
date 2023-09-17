#include "triangle_test.h"
#include "profiler/logger.h"

using namespace ad_astris::renderer::impl;

TriangleTest::TriangleTest(rhi::IEngineRHI* rhi, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager) : _rhi(rhi)
{
	std::vector<rhi::TextureView> swapChainTextureViews;
	_rhi->get_swap_chain_texture_views(swapChainTextureViews);

	rhi::RenderTarget renderTarget1;
	renderTarget1.target = &swapChainTextureViews[0];
	renderTarget1.type = rhi::RenderTargetType::COLOR;
	renderTarget1.initialLayout = rhi::ResourceLayout::UNDEFINED;
	renderTarget1.renderPassLayout = rhi::ResourceLayout::COLOR_ATTACHMENT;
	renderTarget1.finalLayout = rhi::ResourceLayout::PRESENT_ATTACHMENT;
	renderTarget1.loadOp = rhi::LoadOp::CLEAR;
	renderTarget1.storeOp = rhi::StoreOp::STORE;

	rhi::RenderTarget renderTarget2 = renderTarget1;
	renderTarget2.target = &swapChainTextureViews[1];

	rhi::RenderTarget renderTarget3 = renderTarget1;
	renderTarget3.target = &swapChainTextureViews[2];

	rhi::RenderBuffer renderBuffer1;
	renderBuffer1.renderTargets.push_back(renderTarget1);

	rhi::RenderBuffer renderBuffer2;
	renderBuffer2.renderTargets.push_back(renderTarget2);

	rhi::RenderBuffer renderBuffer3;
	renderBuffer3.renderTargets.push_back(renderTarget3);
	
	rhi::RenderPassInfo renderPassInfo;
	renderPassInfo.renderBuffers.push_back(renderBuffer1);
	renderPassInfo.renderBuffers.push_back(renderBuffer2);
	renderPassInfo.renderBuffers.push_back(renderBuffer3);
	renderPassInfo.pipelineType = rhi::PipelineType::GRAPHICS;

	_rhi->create_render_pass(&_renderPass, &renderPassInfo);
	LOG_INFO("TriangleTest::TriangleTest(): Created render pass")

	rhi::Shader* vertexShader = shaderManager->load_shader("engine/shaders/triangleVS.hlsl", rhi::ShaderType::VERTEX);
	auto fragmentShader = shaderManager->load_shader("engine/shaders/trianglePS.hlsl", rhi::ShaderType::FRAGMENT);
	
	rhi::GraphicsPipelineInfo pipelineInfo;
	pipelineInfo.renderPass = _renderPass;
	
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

void TriangleTest::draw()
{
	rhi::CommandBuffer cmd;
	_rhi->begin_command_buffer(&cmd);
	_rhi->bind_pipeline(&cmd, &_pipeline);
	rhi::ClearValues clearValues;
	clearValues.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	_rhi->begin_render_pass(&cmd, &_renderPass, clearValues);

	rhi::Viewport viewport;
	viewport.width = 1280;
	viewport.height = 720;
	std::vector<rhi::Viewport> viewports = { viewport };
	_rhi->set_viewports(&cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = 1280;
	scissor.bottom = 720;
	std::vector<rhi::Scissor> scissors = { scissor };
	_rhi->set_scissors(&cmd, scissors);
	
	_rhi->draw(&cmd, 3);
	_rhi->end_render_pass(&cmd);
}
