#include "triangle_test.h"
#include "profiler/logger.h"

using namespace ad_astris::renderer::impl;

TriangleTest::TriangleTest(rhi::IEngineRHI* rhi, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager, uint32_t width, uint32_t height) : _rhi(rhi)
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

	rhi::TextureInfo textureInfo;
	textureInfo.format = rhi::Format::R8G8B8A8_UNORM;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.textureUsage = rhi::ResourceUsage::COLOR_ATTACHMENT | rhi::ResourceUsage::SAMPLED_TEXTURE;
	textureInfo.textureDimension = rhi::TextureDimension::TEXTURE2D;
	textureInfo.layersCount = 1;
	textureInfo.mipLevels = 1;
	textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
	textureInfo.samplesCount = rhi::SampleCount::BIT_1;
	rhi->create_texture(&_outputTexture, &textureInfo);

	rhi::TextureViewInfo textureViewInfo;
	textureViewInfo.baseLayer = 0;
	textureViewInfo.textureAspect = rhi::TextureAspect::COLOR;
	textureViewInfo.baseMipLevel = 0;
	rhi->create_texture_view(&_outputTextureView, &textureViewInfo, &_outputTexture);
}

void TriangleTest::draw(rhi::CommandBuffer cmd)
{
	auto pipelineBarrier = rhi::PipelineBarrier::set_texture_barrier(&_outputTexture, rhi::ResourceLayout::UNDEFINED, rhi::ResourceLayout::COLOR_ATTACHMENT);
	std::vector<rhi::PipelineBarrier> pipelineBarriers = { pipelineBarrier };
	_rhi->add_pipeline_barriers(&cmd, pipelineBarriers);

	rhi::RenderTarget renderTarget;
	renderTarget.target = &_outputTextureView;
	renderTarget.storeOp = rhi::StoreOp::STORE;
	renderTarget.loadOp = rhi::LoadOp::CLEAR;
	renderTarget.clearValue.color = { 0.5f, 0.5f, 0.5f, 1.0f };
	
	rhi::RenderingBeginInfo beginInfo;
	beginInfo.renderTargets.push_back(renderTarget);
	
	_rhi->begin_rendering(&cmd, &beginInfo);
	_rhi->bind_pipeline(&cmd, &_pipeline);

	_rhi->draw(&cmd, 3);

	_rhi->end_rendering(&cmd);

	pipelineBarrier = rhi::PipelineBarrier::set_texture_barrier(&_outputTexture, rhi::ResourceLayout::COLOR_ATTACHMENT, rhi::ResourceLayout::SHADER_READ);
	pipelineBarriers.clear();
	pipelineBarriers.push_back(pipelineBarrier);
	_rhi->add_pipeline_barriers(&cmd, pipelineBarriers);
}
