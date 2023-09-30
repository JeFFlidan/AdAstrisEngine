#include "deferred_lighting.h"
#include "shader_interop_renderer.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void GBuffer::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	rendererResourceManager->allocate_color_attachment("gAlbedo", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_color_attachment("gNormal", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_color_attachment("gSurface", _mainWindow->get_width(), _mainWindow->get_height());
	//rendererResourceManager->allocate_color_attachment("gVelocity", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_depth_stencil_attachment("gDepthStencil", _mainWindow->get_width(), _mainWindow->get_height());
	rendererResourceManager->allocate_texture_view("gAlbedo", "gAlbedo");
	rendererResourceManager->allocate_texture_view("gNormal", "gNormal");
	rendererResourceManager->allocate_texture_view("gSurface", "gSurface");
	//rendererResourceManager->allocate_texture_view("gVelocity", "gVelocity");
	rendererResourceManager->allocate_texture_view("gDepthStencil", "gDepthStencil");
	
	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("gBuffer", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("gAlbedo");
	renderPass->add_color_output("gNormal");
	renderPass->add_color_output("gSurface");
	//renderPass->add_color_output("gVelocity");
	renderPass->set_depth_stencil_output("gDepthStencil");
	renderPass->set_executor(this);

	_pipelineManager->bind_render_pass_to_pipeline(renderPass, rcore::BuiltinPipelineType::GBUFFER);
}

void GBuffer::execute(rhi::CommandBuffer* cmd)
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
	_rhi->bind_pipeline(cmd, _pipelineManager->get_builtin_pipeline(rcore::BuiltinPipelineType::GBUFFER));
	rhi::Buffer* vertexBuffer = _sceneManager->get_vertex_buffer_f32pntc();
	rhi::Buffer* indexBuffer = _sceneManager->get_index_buffer_f32pntc();
	_rhi->bind_vertex_buffer(cmd, vertexBuffer);
	_rhi->bind_index_buffer(cmd, indexBuffer);
	//_rhi->draw_indexed(cmd, 27018, 1, 0, 0, 0);

	IndirectBufferDesc* indirectBufferDesc = _sceneManager->get_indirect_buffer_desc();
	rhi::Buffer* indirectBuffer = indirectBufferDesc->get_indirect_buffer();
	uint32_t offset = 0;
	for (uint32_t i = 0; i != indirectBufferDesc->get_indirect_command_count(); ++i)
	{
		_rhi->draw_indexed_indirect(cmd, indirectBuffer, offset, 1, sizeof(DrawIndexedIndirectCommand));
		uint32_t batchInstanceCount = indirectBufferDesc->get_batch_instance_count(i);
		offset += batchInstanceCount * sizeof(DrawIndexedIndirectCommand);
	}
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

	_pipelineManager->bind_render_pass_to_pipeline(renderPass, rcore::BuiltinPipelineType::DEFERRED_LIGHTING);

	_attachments.gAlbedoIndex = _rhi->get_descriptor_index(rendererResourceManager->get_texture_view("gAlbedo"));
	_attachments.gNormalIndex = _rhi->get_descriptor_index(rendererResourceManager->get_texture_view("gNormal"));
	_attachments.gDepthIndex = _rhi->get_descriptor_index(rendererResourceManager->get_texture_view("gDepthStencil"));
	_attachments.gSurfaceIndex = _rhi->get_descriptor_index(rendererResourceManager->get_texture_view("gSurface"));
}

void DeferredLighting::execute(rhi::CommandBuffer* cmd)
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

	rhi::Pipeline* pipeline = _pipelineManager->get_builtin_pipeline(rcore::BuiltinPipelineType::DEFERRED_LIGHTING);
	_rhi->bind_pipeline(cmd, pipeline);
	_rhi->push_constants(cmd, pipeline, &_attachments);

	rhi::Buffer* vertexBufffer = _sceneManager->get_output_plane_vertex_buffer();
	_rhi->bind_vertex_buffer(cmd, vertexBufffer);
	
	_rhi->draw(cmd, 6);
}
