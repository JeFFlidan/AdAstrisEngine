#include "temporal_filter.h"
#include "renderer/public/attachment_name.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void TemporalFilter::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment(AttachmentName::TAA_OUTPUT, IMAGE_WIDTH, IMAGE_HEIGHT);
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view(AttachmentName::TAA_OUTPUT, AttachmentName::TAA_OUTPUT);

	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("TAA", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_input(AttachmentName::OIT_COLOR);
	renderPass->add_color_input(AttachmentName::OIT_VELOCITY);
	renderPass->add_color_input(AttachmentName::DEFERRED_LIGHTING_OUTPUT);
	renderPass->add_color_output(AttachmentName::TAA_OUTPUT);
	renderPass->set_executor(this);
}

void TemporalFilter::execute(rhi::CommandBuffer* cmd)
{
	
}
