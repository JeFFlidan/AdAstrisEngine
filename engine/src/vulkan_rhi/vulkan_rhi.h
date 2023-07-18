#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_command_manager.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_swap_chain.h"
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include <VkBootstrap.h>

#include <vector>
#include <memory>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanRHI : public rhi::IEngineRHI
	{
		public:
			// TODO Create custom destructor
			~VulkanRHI() final override = default;

			virtual void init(void* window, io::FileSystem* fileSystem) final override;
			virtual void cleanup() final override;

			virtual void create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info) final override;
			virtual void destroy_swap_chain(rhi::SwapChain* swapChain) final override;
		
			virtual void create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data = nullptr) final override;

			virtual void update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data) final override;
			virtual void create_texture(rhi::Texture* texture, rhi::TextureInfo* texInfo) final override;
			virtual void create_texture_view(
				rhi::TextureView* textureView,
				rhi::TextureViewInfo* viewInfo,
				rhi::Texture* texture) final override;
			virtual void create_sampler(rhi::Sampler* sampler, rhi::SamplerInfo* sampInfo) final override;
			virtual void create_shader(rhi::Shader* shader, rhi::ShaderInfo* shaderInfo) final override;
			virtual void create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo) final override;
			virtual void create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info) final override;
			virtual void create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info) final override;

			virtual void begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType = rhi::QueueType::GRAPHICS) final override;
			virtual void wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd) final override;
			virtual void submit(rhi::QueueType queueType = rhi::QueueType::GRAPHICS) final override;

			virtual void copy_buffer(
				rhi::CommandBuffer* cmd,
				rhi::Buffer* srcBuffer,
				rhi::Buffer* dstBuffer,
				uint32_t size = 0,
				uint32_t srcOffset = 0,
				uint32_t dstOffset = 0) final override;
			virtual void blit_texture(
				rhi::CommandBuffer* cmd,
				rhi::Texture* srcTexture,
				rhi::Texture* dstTexture,
				std::array<int32_t, 3>& srcOffset,
				std::array<int32_t, 3>& dstOffset,
				uint32_t srcMipLevel = 0,
				uint32_t dstMipLevel = 0,
				uint32_t srcBaseLayer = 0,
				uint32_t dstBaseLayer = 0) final override;
			virtual void copy_buffer_to_texture(
				rhi::CommandBuffer* cmd,
				rhi::Buffer* srcBuffer,
				rhi::Texture* dstTexture,
				rhi::ResourceUsage textureUsage) final override;
			virtual void set_viewport(rhi::CommandBuffer* cmd, float width, float height) final override;
			virtual void set_scissor(rhi::CommandBuffer* cmd, uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) final override;
			virtual void bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void bind_pipeline(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline) final override;
			virtual void begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass, rhi::ClearValues& clearValues) final override;
			virtual void end_render_pass(rhi::CommandBuffer* cmd) final override;
			virtual void draw(rhi::CommandBuffer* cmd, uint64_t vertexCount) final override;
			virtual void draw_indexed(
				rhi::CommandBuffer* cmd,
				uint32_t indexCount,
				uint32_t instanceCount,
				uint32_t firstIndex,
				int32_t vertexOffset,
				uint32_t firstInstance) final override;
			virtual void draw_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) final override;
			virtual void draw_indexed_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) final override;
			virtual void dispatch(rhi::CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) final override;
			virtual void fill_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data) final override;
			virtual void add_pipeline_barriers(rhi::CommandBuffer* cmd, std::vector<rhi::PipelineBarrier>& barriers) final override;
		
			// Only for tests. Will be removed in the final implementation
			VkInstance get_instance() { return _instance; }
			VulkanDevice* get_device() { return _vulkanDevice.get(); }
			VmaAllocator get_allocator() { return _allocator; }
			VkDebugUtilsMessengerEXT get_messenger() { return _debugMessenger; }
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			VkInstance _instance{ VK_NULL_HANDLE };
			VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
			VmaAllocator _allocator;
			std::unique_ptr<VulkanDevice> _vulkanDevice{ nullptr };
			std::unique_ptr<VulkanCommandManager> _cmdManager{ nullptr };
			std::unique_ptr<VulkanSwapChain> _swapChain{ nullptr };
			VkPipelineCache _pipelineCache;

			std::vector<std::unique_ptr<VulkanPipeline>> _vulkanPipelines;
			std::vector<std::unique_ptr<VulkanShader>> _vulkanShaders;
			std::vector<std::unique_ptr<VulkanRenderPass>> _vulkanRenderPasses;
			std::vector<std::unique_ptr<VkSampler>> _vulkanSamplers;
			std::vector<std::unique_ptr<VkImageView>> _vulkanImageViews;
			std::vector<std::unique_ptr<VulkanTexture>> _vulkanTextures;
			std::vector<std::unique_ptr<VulkanBuffer>> _vulkanBuffers;

			vkb::Instance create_instance();
			void create_allocator();
			void create_pipeline_cache();
			void save_pipeline_cache();
	};
}
