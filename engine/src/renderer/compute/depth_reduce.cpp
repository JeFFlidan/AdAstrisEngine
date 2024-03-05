#include "depth_reduce.h"
#include "shader_interop_renderer.h"
#include "renderer/utils.h"

using namespace ad_astris;
using namespace renderer::impl;

void DepthReduce::prepare_render_pass()
{
	rcore::IRenderPass* renderPass = RENDER_GRAPH()->add_new_pass("DepthReducing", rcore::RenderGraphQueue::COMPUTE);
	DepthPyramid& depthPyramid = Utils::get_depth_pyramid(STATIC_OPAQUE_FILTER, ecore::MAIN_CAMERA);
	renderPass->add_storage_texture_output(depthPyramid.get_depth_pyramid_name());
	renderPass->set_executor(this);
}

void DepthReduce::execute(rhi::CommandBuffer* cmd)
{
	DepthPyramid& depthPyramid = Utils::get_depth_pyramid(STATIC_OPAQUE_FILTER, ecore::MAIN_CAMERA);
	rhi::Texture* gDepthStencilTexture = RENDERER_RESOURCE_MANAGER()->get_texture("gDepthStencil");
	rhi::TextureView* gDepthStencilView = RENDERER_RESOURCE_MANAGER()->get_texture_view("gDepthStencil");
	uint32_t mipLevels = depthPyramid.get_mip_levels();
	
	rhi::Pipeline* pipeline = PIPELINE_MANAGER()->get_builtin_pipeline(rcore::BuiltinPipelineType::DEPTH_REDUCE);
	RHI()->bind_pipeline(cmd, pipeline);

	rhi::PipelineBarrier barrier = rhi::PipelineBarrier::set_texture_barrier(
		gDepthStencilTexture,
		rhi::ResourceLayout::DEPTH_STENCIL,
		rhi::ResourceLayout::SHADER_READ);
	RHI()->add_pipeline_barriers(cmd, { barrier });
	
	DepthReduceData depthReduceData;
	for (uint32_t i = 0; i != mipLevels; ++i)
	{
		depthReduceData.inDepthTextureIndex = RHI()->get_descriptor_index(!i ? gDepthStencilView : depthPyramid.get_mipmap(i - 1));
		depthReduceData.outDepthTextureIndex = RHI()->get_descriptor_index(depthPyramid.get_mipmap(i));
		depthReduceData.levelWidth = std::max(1u, depthPyramid.get_width() >> i);
		depthReduceData.levelHeight = std::max(1u, depthPyramid.get_height() >> i);
		RHI()->push_constants(cmd, pipeline, &depthReduceData);
		RHI()->dispatch(
			cmd,
			Utils::get_group_count(depthReduceData.levelWidth, DEPTH_REDUCE_GROUP_SIZE),
			Utils::get_group_count(depthReduceData.levelHeight, DEPTH_REDUCE_GROUP_SIZE),
			1);
		barrier = rhi::PipelineBarrier::set_texture_barrier(
			depthPyramid.get_texture(),
			rhi::ResourceLayout::GENERAL | rhi::ResourceLayout::SHADER_WRITE,
			rhi::ResourceLayout::SHADER_READ,
			i, 1);
		RHI()->add_pipeline_barriers(cmd, { barrier });
	}

	barrier = rhi::PipelineBarrier::set_texture_barrier(
		gDepthStencilTexture,
		rhi::ResourceLayout::SHADER_READ,
		rhi::ResourceLayout::DEPTH_STENCIL);
	RHI()->add_pipeline_barriers(cmd, { barrier });
}
