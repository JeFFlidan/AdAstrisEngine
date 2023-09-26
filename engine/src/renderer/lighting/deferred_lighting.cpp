#include "deferred_lighting.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void GBuffer::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("gAlbedo", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_color_attachment("gNormal", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_color_attachment("gSurface", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_color_attachment("gVelocity", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_depth_stencil_attachment("gDepthStencil", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("gAlbedo", "gAlbedo");
	rendererResourceManager->allocate_texture_view("gNormal", "gNormal");
	rendererResourceManager->allocate_texture_view("gSurface", "gSurface");
	rendererResourceManager->allocate_texture_view("gVelocity", "gVelocity");
	rendererResourceManager->allocate_texture_view("gDepthStencil", "gDepthStencil");
	
	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("gBuffer", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("gAlbedo");
	renderPass->add_color_output("gNormal");
	renderPass->add_color_output("gSurface");
	renderPass->add_color_output("gVelocity");
	renderPass->set_depth_stencil_output("gDepthStencil");
	renderPass->set_executor(this);
}

void GBuffer::execute(rhi::CommandBuffer* cmd)
{
	
}

void DeferredLighting::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("DeferredLightingOutput", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("DeferredLightingOutput", "DeferredLightingOutput");

	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("DeferredLighting", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_input("gAlbedo");
	renderPass->add_color_input("gNormal");
	renderPass->add_color_input("gSurface");
	renderPass->set_depth_stencil_input("gDepthStencil");
	renderPass->add_color_output("DeferredLightingOutput");
	renderPass->set_executor(this);
}

void DeferredLighting::execute(rhi::CommandBuffer* cmd)
{
	
}
