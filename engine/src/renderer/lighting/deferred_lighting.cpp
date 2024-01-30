#include "deferred_lighting.h"
#include "shader_interop_renderer.h"
#include "profiler/profiler.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void GBuffer::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gAlbedo", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gNormal", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gSurface", _mainWindow->get_width(), _mainWindow->get_height());
	//rendererResourceManager->allocate_color_attachment("gVelocity", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_depth_stencil_attachment("gDepthStencil", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gAlbedo", "gAlbedo");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gNormal", "gNormal");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gSurface", "gSurface");
	//rendererResourceManager->allocate_texture_view("gVelocity", "gVelocity");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gDepthStencil", "gDepthStencil");
	
	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("gBuffer", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("gAlbedo");
	renderPass->add_color_output("gNormal");
	renderPass->add_color_output("gSurface");
	//renderPass->add_color_output("gVelocity");
	renderPass->set_depth_stencil_output("gDepthStencil");
	renderPass->set_executor(this);

	PIPELINE_MANAGER()->bind_render_pass_to_pipeline(renderPass, rcore::BuiltinPipelineType::GBUFFER);
}

void GBuffer::execute(rhi::CommandBuffer* cmd)
{
	auto rangeID = profiler::Profiler::begin_gpu_range("GBuffer", *cmd);
	rhi::Viewport viewport;
	viewport.width = _mainWindow->get_width();
	viewport.height = _mainWindow->get_height();
	std::vector<rhi::Viewport> viewports = { viewport };
	RHI()->set_viewports(cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = _mainWindow->get_width();
	scissor.bottom = _mainWindow->get_height();
	std::vector<rhi::Scissor> scissors = { scissor };
	RHI()->set_scissors(cmd, scissors);
	RHI()->bind_pipeline(cmd, PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::GBUFFER));
	rhi::Buffer* vertexBuffer = SCENE_MANAGER()->get_vertex_buffer_f32pntc();
	rhi::Buffer* indexBuffer = SCENE_MANAGER()->get_index_buffer_f32pntc();
	RHI()->bind_vertex_buffer(cmd, vertexBuffer);
	RHI()->bind_index_buffer(cmd, indexBuffer);
	//_rhi->draw_indexed(cmd, 27018, 1, 0, 0, 0);

	IndirectBufferDesc* indirectBufferDesc = SCENE_MANAGER()->get_indirect_buffer_desc();
	rhi::Buffer* indirectBuffer = indirectBufferDesc->get_indirect_buffer();
	uint32_t offset = 0;
	for (uint32_t i = 0; i != indirectBufferDesc->get_indirect_command_count(); ++i)
	{
		RHI()->draw_indexed_indirect(cmd, indirectBuffer, offset, 1, sizeof(DrawIndexedIndirectCommand));
		uint32_t batchInstanceCount = indirectBufferDesc->get_batch_instance_count(i);
		offset += batchInstanceCount * sizeof(DrawIndexedIndirectCommand);
	}

	profiler::Profiler::end_gpu_range(rangeID);
}

void DeferredLighting::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("DeferredLightingOutput", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("DeferredLightingOutput", "DeferredLightingOutput");

	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("DeferredLighting", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_input("gAlbedo");
	renderPass->add_color_input("gNormal");
	renderPass->add_color_input("gSurface");
	renderPass->set_depth_stencil_input("gDepthStencil");
	renderPass->add_color_output("DeferredLightingOutput");
	renderPass->set_executor(this);

	PIPELINE_MANAGER()->bind_render_pass_to_pipeline(renderPass, rcore::BuiltinPipelineType::DEFERRED_LIGHTING);

	_attachments.gAlbedoIndex = RHI()->get_descriptor_index(RENDERER_RESOURCE_MANAGER()->get_texture_view("gAlbedo"));
	_attachments.gNormalIndex = RHI()->get_descriptor_index(RENDERER_RESOURCE_MANAGER()->get_texture_view("gNormal"));
	_attachments.gDepthIndex = RHI()->get_descriptor_index(RENDERER_RESOURCE_MANAGER()->get_texture_view("gDepthStencil"));
	_attachments.gSurfaceIndex = RHI()->get_descriptor_index(RENDERER_RESOURCE_MANAGER()->get_texture_view("gSurface"));
}

void DeferredLighting::execute(rhi::CommandBuffer* cmd)
{
	profiler::RangeID rangeID = profiler::Profiler::begin_gpu_range("Deferred lighting", *cmd);
	rhi::Viewport viewport;
	viewport.width = _mainWindow->get_width();
	viewport.height = _mainWindow->get_height();
	std::vector<rhi::Viewport> viewports = { viewport };
	RHI()->set_viewports(cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = _mainWindow->get_width();
	scissor.bottom = _mainWindow->get_height();
	std::vector<rhi::Scissor> scissors = { scissor };
	RHI()->set_scissors(cmd, scissors);

	rhi::Pipeline* pipeline = PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::DEFERRED_LIGHTING);
	RHI()->bind_pipeline(cmd, pipeline);
	RHI()->push_constants(cmd, pipeline, &_attachments);

	rhi::Buffer* vertexBufffer = SCENE_MANAGER()->get_output_plane_vertex_buffer();
	RHI()->bind_vertex_buffer(cmd, vertexBufffer);
	
	RHI()->draw(cmd, 6);
	profiler::Profiler::end_gpu_range(rangeID);
}
