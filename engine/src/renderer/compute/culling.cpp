#include "culling.h"
#include "renderer/common.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void Culling::prepare_render_pass()
{
	CullingSubmanager* cullingSubmanager = SCENE_MANAGER()->get_culling_submanager();
	cullingSubmanager->add_scene_entity_filter(STATIC_OPAQUE_FILTER);
	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("OpaqueModelsCulling", rcore::RenderGraphQueue::COMPUTE);
	renderPass->add_storage_buffer_read_write_output(cullingSubmanager->get_indirect_buffer_name(STATIC_OPAQUE_FILTER));
	renderPass->add_storage_buffer_read_write_output(cullingSubmanager->get_model_instance_id_buffer_name(STATIC_OPAQUE_FILTER));
	renderPass->set_executor(this);
}

void Culling::execute(rhi::CommandBuffer* cmd)
{
	rhi::Pipeline* pipeline = PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::CULLING);
	RHI()->bind_pipeline(cmd, pipeline);

	CullingSubmanager* cullingSubmanager = SCENE_MANAGER()->get_culling_submanager();
	IndirectBuffers indirectBuffers = cullingSubmanager->get_scene_indirect_buffers(STATIC_OPAQUE_FILTER, ecore::MAIN_CAMERA);
	rhi::Buffer* cullingIndicesBuffer = cullingSubmanager->get_culling_indices_buffer(STATIC_OPAQUE_FILTER);
	IndirectBufferIndices indirectBufferIndices;
	indirectBufferIndices.indirectCommandsBufferIndex = RHI()->get_descriptor_index(indirectBuffers.indirectBuffer);
	indirectBufferIndices.rendererModelInstanceIdBufferIndex = RHI()->get_descriptor_index(indirectBuffers.modelInstanceIDBuffer);
	indirectBufferIndices.cullingInstanceIndicesBufferIndex = RHI()->get_descriptor_index(cullingIndicesBuffer);
	indirectBufferIndices.cullingParamsBufferIndex = RHI()->get_descriptor_index(cullingSubmanager->get_culling_params_buffer());
	indirectBufferIndices.cullingParamsIndex = indirectBuffers.cullingParamsIndex;
	RHI()->push_constants(cmd, pipeline, &indirectBufferIndices);
	
	auto getGroupCount = [&](const ecs::IEntityFilter& filter)->uint32_t
	{
		return cullingSubmanager->get_instance_count(filter) / CULLING_GROUP_SIZE + 1;
	};
	
	RHI()->dispatch(cmd, getGroupCount(STATIC_OPAQUE_FILTER), 1, 1);
}
