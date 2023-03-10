#pragma once

#include "resources.h"

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
		
			virtual void create_buffer(Buffer* buffer, BufferInfo* info, uint64_t size, void* data = nullptr) = 0;
			virtual void update_buffer_data(Buffer* buffer, uint64_t size, void* data) = 0;
			virtual void create_texture(Texture* texture, TextureInfo* info) = 0;
			virtual void create_texture_view(TextureView* textureView, TextureViewInfo* info, Texture* texture) = 0;
			virtual void create_sampler(Sampler* sampler, SamplerInfo* info) = 0;
			virtual void create_shader(Shader* shader, ShaderInfo* shaderInfo) = 0;
			virtual void create_render_pass(RenderPass* renderPass, RenderPassInfo* passInfo) = 0;
			virtual void create_graphics_pipeline(Pipeline* pipeline, GraphicsPipelineInfo* info) = 0;
			virtual void create_compute_pipeline(Pipeline* pipeline, ComputePipelineInfo* info) = 0;
	};
}
