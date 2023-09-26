#include "temporal_filter.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void TemporalFilter::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("TAAOutput", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("TAAOutput", "TAAOutput");

	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("TAA", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_input("OITColor");
	renderPass->add_color_input("OITVelocity");
	renderPass->add_color_input("DeferredLightingOutput");
	renderPass->add_color_output("TAAOutput");
	renderPass->set_executor(this);
}

void TemporalFilter::execute(rhi::CommandBuffer* cmd)
{
	
}
