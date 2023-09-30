#include "occlusion_culling.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void OcclusionCulling::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	IndirectBufferDesc* indirectBufferDesc = _sceneManager->get_indirect_buffer_desc();
	rcore::IRenderPass* renderPass = renderGraph->add_new_pass("OcclusionCulling", rcore::RenderGraphQueue::COMPUTE);
	renderPass->add_storage_buffer_read_write_output(indirectBufferDesc->get_indirect_buffer_name());
	renderPass->set_executor(this);
}

void OcclusionCulling::execute(rhi::CommandBuffer* cmd)
{
	rhi::Pipeline* pipeline = _pipelineManager->get_builtin_pipeline(rcore::BuiltinPipelineType::OCCLUSION_CULLING);
	_rhi->bind_pipeline(cmd, pipeline);

	IndirectBufferDesc* indirectBufferDesc = _sceneManager->get_indirect_buffer_desc();
	IndirectBufferIndices indirectBufferIndices;
	indirectBufferIndices.indirectCommandsBufferIndex = _rhi->get_descriptor_index(indirectBufferDesc->get_indirect_buffer());
	indirectBufferIndices.cullingInstanceIndicesBufferIndex = _rhi->get_descriptor_index(indirectBufferDesc->get_culling_instance_indices_buffer());
	_rhi->push_constants(cmd, pipeline, &indirectBufferIndices);

	_rhi->dispatch(cmd, indirectBufferDesc->get_all_instance_count() / 256 + 1, 1, 1);
}
