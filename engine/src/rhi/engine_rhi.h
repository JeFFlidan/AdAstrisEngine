#pragma once

#include "resources.h"
#include "file_system/file_system.h"
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace ad_astris::vulkan
{
	class VulkanDevice;
}

namespace ad_astris::rhi
{
	class IEngineRHI
	{
		public:
			virtual ~IEngineRHI() = default;
		
			virtual void init(void* window, io::FileSystem* fileSystem) = 0;
			virtual void cleanup() = 0;

			virtual void create_swap_chain(SwapChain* swapChain, SwapChainInfo* info) = 0;
			virtual void destroy_swap_chain(SwapChain* swapChain) = 0;

			// Can create an empty buffer or buffer with data
			virtual void create_buffer(Buffer* buffer, BufferInfo* info, void* data = nullptr) = 0;
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

			// If size = 0 (default value), method will copy whole srcBuffer to dstBuffer
			virtual void copy_buffer(
				CommandBuffer* cmd,
				Buffer* srcBuffer,
				Buffer* dstBuffer,
				uint32_t size = 0,
				uint32_t srcOffset = 0,
				uint32_t dstOffset = 0) = 0;
			// srcTexture should have ResourceLayout TRANSFER_SRC, dstTexture should have ResourceLayout TRANSFER_DST
			virtual void blit_texture(
				CommandBuffer* cmd,
				Texture* srcTexture,
				Texture* dstTexture,
				std::array<int32_t, 3>& srcOffset,
				std::array<int32_t, 3>& dstOffset,
				uint32_t srcMipLevel = 0,
				uint32_t dstMipLevel = 0,
				uint32_t srcBaseLayer = 0,
				uint32_t dstBaseLayer = 0) = 0;
			virtual void copy_buffer_to_texture(
				CommandBuffer* cmd,
				Buffer* srcBuffer,
				Texture* dstTexture,
				ResourceUsage textureUsage) = 0;
			virtual void set_viewport(CommandBuffer* cmd, float width, float height) = 0;
			virtual void set_scissor(CommandBuffer* cmd, uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) = 0;
			// Can bind only one vertex buffer.
			virtual void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_pipeline(CommandBuffer* cmd, Pipeline* pipeline) = 0;
			virtual void begin_render_pass(CommandBuffer* cmd, RenderPass* renderPass, ClearValues& clearValues) = 0;
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


			// ONLY FOR TESTS
			virtual VkInstance get_instance() = 0;
			virtual vulkan::VulkanDevice* get_device() = 0;
			virtual VmaAllocator get_allocator() = 0;
			virtual VkDebugUtilsMessengerEXT get_messenger() = 0;
	};
}
