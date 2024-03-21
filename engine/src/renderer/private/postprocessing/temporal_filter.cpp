#include "temporal_filter.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void TemporalFilter::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("TAAOutput", IMAGE_WIDTH, IMAGE_HEIGHT);
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("TAAOutput", "TAAOutput");

	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("TAA", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_input("OITColor");
	renderPass->add_color_input("OITVelocity");
	renderPass->add_color_input("DeferredLightingOutput");
	renderPass->add_color_output("TAAOutput");
	renderPass->set_executor(this);
}

void TemporalFilter::execute(rhi::CommandBuffer* cmd)
{
	
}
