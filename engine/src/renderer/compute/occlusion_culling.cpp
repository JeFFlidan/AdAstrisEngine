#include "occlusion_culling.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void OcclusionCulling::prepare_render_pass()
{
	IndirectBufferDesc* indirectBufferDesc = SCENE_MANAGER()->get_indirect_buffer_desc();
	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("OcclusionCulling", rcore::RenderGraphQueue::COMPUTE);
	renderPass->add_storage_buffer_read_write_output(indirectBufferDesc->get_indirect_buffer_name());
	renderPass->set_executor(this);
}

void OcclusionCulling::execute(rhi::CommandBuffer* cmd)
{
	rhi::Pipeline* pipeline = PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::OCCLUSION_CULLING);
	RHI()->bind_pipeline(cmd, pipeline);

	IndirectBufferDesc* indirectBufferDesc = SCENE_MANAGER()->get_indirect_buffer_desc();
	IndirectBufferIndices indirectBufferIndices;
	indirectBufferIndices.indirectCommandsBufferIndex = RHI()->get_descriptor_index(indirectBufferDesc->get_indirect_buffer());
	indirectBufferIndices.cullingInstanceIndicesBufferIndex = RHI()->get_descriptor_index(indirectBufferDesc->get_culling_instance_indices_buffer());
	RHI()->push_constants(cmd, pipeline, &indirectBufferIndices);

	RHI()->dispatch(cmd, indirectBufferDesc->get_all_instance_count() / 256 + 1, 1, 1);
}
