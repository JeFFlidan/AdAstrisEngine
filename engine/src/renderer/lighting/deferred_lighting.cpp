﻿#include "deferred_lighting.h"
#include "shader_interop_renderer.h"
#include "profiler/profiler.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void GBuffer::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gAlbedo", IMAGE_WIDTH, IMAGE_HEIGHT);
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gNormal", IMAGE_WIDTH, IMAGE_HEIGHT);
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("gSurface", IMAGE_WIDTH, IMAGE_HEIGHT);
	//rendererResourceManager->allocate_color_attachment("gVelocity", _mainWindow->get_width(), _mainWindow->get_height());
	RENDERER_RESOURCE_MANAGER()->allocate_depth_stencil_attachment("gDepthStencil", IMAGE_WIDTH, IMAGE_HEIGHT);
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gAlbedo", "gAlbedo");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gNormal", "gNormal");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gSurface", "gSurface");
	//rendererResourceManager->allocate_texture_view("gVelocity", "gVelocity");
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view("gDepthStencil", "gDepthStencil");

	CullingSubmanager* cullingSubmanager = SCENE_MANAGER()->get_culling_submanager();
	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("gBuffer", rcore::RenderGraphQueue::GRAPHICS);
	renderPass->add_color_output("gAlbedo");
	renderPass->add_color_output("gNormal");
	renderPass->add_color_output("gSurface");
	//renderPass->add_color_output("gVelocity");
	renderPass->set_depth_stencil_output("gDepthStencil");
	renderPass->add_indirect_buffer_input(cullingSubmanager->get_indirect_buffer_name(STATIC_OPAQUE_FILTER));
	renderPass->add_storage_buffer_read_only_input(cullingSubmanager->get_model_instance_id_buffer_name(STATIC_OPAQUE_FILTER));
	renderPass->set_executor(this);

	PIPELINE_MANAGER()->bind_render_pass_to_pipeline(renderPass, rcore::BuiltinPipelineType::GBUFFER);
}

void GBuffer::execute(rhi::CommandBuffer* cmd)
{
	auto rangeID = profiler::Profiler::begin_gpu_range("GBuffer", *cmd);
	rhi::Viewport viewport;
	viewport.width = IMAGE_WIDTH;
	viewport.height = IMAGE_HEIGHT;
	std::vector<rhi::Viewport> viewports = { viewport };
	RHI()->set_viewports(cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = (int32_t)IMAGE_WIDTH;
	scissor.bottom = (int32_t)IMAGE_HEIGHT;
	std::vector<rhi::Scissor> scissors = { scissor };
	RHI()->set_scissors(cmd, scissors);
	RHI()->bind_pipeline(cmd, PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::GBUFFER));
	rhi::Buffer* vertexBuffer = SCENE_MANAGER()->get_vertex_buffer_f32pntc();
	rhi::Buffer* indexBuffer = SCENE_MANAGER()->get_index_buffer_f32pntc();
	RHI()->bind_vertex_buffer(cmd, vertexBuffer);
	RHI()->bind_index_buffer(cmd, indexBuffer);
	//_rhi->draw_indexed(cmd, 27018, 1, 0, 0, 0);

	CullingSubmanager* cullingSubmanager = SCENE_MANAGER()->get_culling_submanager();
	rhi::Buffer* indirectBuffer = cullingSubmanager->get_scene_indirect_buffers(STATIC_OPAQUE_FILTER, ecore::MAIN_CAMERA).indirectBuffer;
	uint32_t offset = 0;
	uint32_t indirectCommandCount = cullingSubmanager->get_indirect_command_count(STATIC_OPAQUE_FILTER);
	for (uint32_t i = 0; i != indirectCommandCount; ++i)
	{
		RHI()->draw_indexed_indirect(cmd, indirectBuffer, offset, 1, sizeof(DrawIndexedIndirectCommand));
		offset += (i + 1) * sizeof(DrawIndexedIndirectCommand);
	}

	profiler::Profiler::end_gpu_range(rangeID);
}

void DeferredLighting::prepare_render_pass()
{
	RENDERER_RESOURCE_MANAGER()->allocate_color_attachment("DeferredLightingOutput", IMAGE_WIDTH, IMAGE_HEIGHT);
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
	viewport.width = IMAGE_WIDTH;
	viewport.height = IMAGE_HEIGHT;
	std::vector<rhi::Viewport> viewports = { viewport };
	RHI()->set_viewports(cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = (int32_t)IMAGE_WIDTH;
	scissor.bottom = (int32_t)IMAGE_HEIGHT;
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
