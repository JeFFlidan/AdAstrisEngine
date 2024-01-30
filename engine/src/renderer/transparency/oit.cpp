#include "oit.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void OITGeometry::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer("OITNodes", 500);	// must change

	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("OITGeometry", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_storage_buffer_read_write_output("OITNodes");
	renderPass->set_executor(this);
}

void OITGeometry::execute(rhi::CommandBuffer* cmd)
{
	
}

void OIT::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("OITColor", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("OITColor", "OITColor");

	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("OITVelocity", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("OITVelocity", "OITVelocity");

	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("OIT", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_storage_buffer_read_only_input("OITNodes");
	renderPass->add_color_output("OITVelocity");
	renderPass->add_color_output("OITColor");
	renderPass->set_executor(this);
}

void OIT::execute(rhi::CommandBuffer* cmd)
{
	
}
