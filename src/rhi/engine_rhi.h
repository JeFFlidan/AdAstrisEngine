#pragma once

#include "resources.h"
#include "vector"

namespace ad_astris::rhi
{
	class IEngineRHI
	{
		public:
			virtual ~IEngineRHI() = default;
		
			virtual void init(void* window) = 0;
			virtual void cleanup() = 0;

			virtual void create_swap_chain(SwapChain* swapChain, SwapChainInfo* info) = 0;
			virtual void destroy_swap_chain(SwapChain* swapChain) = 0;

			// Can create an empty buffer or buffer with data
			virtual void create_buffer(Buffer* buffer, BufferInfo* info, uint64_t size, void* data = nullptr) = 0;
			virtual void update_buffer_data(Buffer* buffer, uint64_t size, void* data) = 0;
			virtual void create_texture(Texture* texture, TextureInfo* info) = 0;
			virtual void create_texture_view(TextureView* textureView, TextureViewInfo* info, Texture* texture) = 0;
			virtual void create_sampler(Sampler* sampler, SamplerInfo* info) = 0;
			virtual void create_shader(Shader* shader, ShaderInfo* shaderInfo) = 0;
			virtual void create_render_pass(RenderPass* renderPass, RenderPassInfo* passInfo) = 0;
			virtual void create_graphics_pipeline(Pipeline* pipeline, GraphicsPipelineInfo* info) = 0;
			virtual void create_compute_pipeline(Pipeline* pipeline, ComputePipelineInfo* info) = 0;

			virtual void begin_command_buffer(CommandBuffer* cmd, QueueType queueType = QueueType::GRAPHICS) = 0;
			virtual void wait_command_buffer(CommandBuffer* cmd, CommandBuffer* waitForCmd) = 0;
			virtual void submit(QueueType queueType = QueueType::GRAPHICS) = 0;

			virtual void set_viewport(CommandBuffer* cmd, float width, float height) = 0;
			virtual void set_scissor(CommandBuffer* cmd, uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) = 0;
			// Can bind only one vertex buffer.
			virtual void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_pipeline(CommandBuffer* cmd, Pipeline* pipeline) = 0;
			virtual void begin_render_pass(CommandBuffer* cmd, RenderPass* renderPass) = 0;
			virtual void end_render_pass(CommandBuffer* cmd) = 0;
			// One buffer - one object
			virtual void draw(CommandBuffer* cmd, uint64_t vertexCount) = 0;
			virtual void draw_indexed(
				CommandBuffer* cmd,
				uint32_t indexCount,
				uint32_t instanceCount,
				uint32_t firstIndex,
				int32_t vertexOffset,
				uint32_t firstInstance) = 0;
			virtual void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) = 0;
			virtual void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) = 0;
			virtual void dispatch(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
			virtual void fill_buffer(CommandBuffer* cmd, Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data) = 0;
			virtual void add_pipeline_barriers(CommandBuffer* cmd, std::vector<PipelineBarrier>& barriers) = 0;
	};
}
