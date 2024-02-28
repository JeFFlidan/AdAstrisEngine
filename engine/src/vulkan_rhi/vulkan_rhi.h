#pragma once

#include "vulkan_instance.h"
#include "vulkan_surface.h"
#include "vulkan_device.h"
#include "vulkan_command_manager.h"
#include "vulkan_object_pool.h"
#include "vulkan_attachment_manager.h"
#include "vulkan_swap_chain.h"
#include "vulkan_descriptor_manager.h"
#include "vulkan_pipeline_layout_cache.h"
#include "vulkan_pipeline_cache.h"
#include "rhi/engine_rhi.h"
#include "rhi/resources.h"

#include <vector>
#include <memory>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanRHI final : public rhi::RHI
	{
		public:
			virtual void init(rhi::RHIInitContext& initContext) override;
			virtual void cleanup() override;

			virtual void create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info) override;
			virtual void destroy_swap_chain(rhi::SwapChain* swapChain) override;
			virtual void get_swap_chain_texture_views(std::vector<rhi::TextureView>& textureViews) override;

			virtual bool acquire_next_image(uint32_t& nextImageIndex, uint32_t currentFrameIndex) override;
		
			virtual void create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data = nullptr) override;
			virtual void create_buffer(rhi::Buffer* buffer, void* data = nullptr) override;
			virtual void destroy_buffer(rhi::Buffer* buffer) override;

			virtual void update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data) override;
			virtual void create_texture(rhi::Texture* texture, rhi::TextureInfo* texInfo) override;
			virtual void create_texture(rhi::Texture* texture) override;
			virtual void create_texture_view(
				rhi::TextureView* textureView,
				rhi::TextureViewInfo* viewInfo,
				rhi::Texture* texture) override;
			virtual void create_texture_view(rhi::TextureView* textureView, rhi::Texture* texture) override;
			virtual void create_sampler(rhi::Sampler* sampler, rhi::SamplerInfo* sampInfo) override;
			virtual void create_shader(rhi::Shader* shader, rhi::ShaderInfo* shaderInfo) override;
			virtual void create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo) override;
			virtual void create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info) override;
			virtual void create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info) override;

			virtual uint32_t get_descriptor_index(rhi::Buffer* buffer) override;
			virtual uint32_t get_descriptor_index(rhi::TextureView* textureView) override;
			virtual uint32_t get_descriptor_index(rhi::Sampler* sampler) override;
			virtual void bind_uniform_buffer(rhi::Buffer* buffer, uint32_t slot, uint32_t size, uint32_t offset) override;

			virtual void begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType = rhi::QueueType::GRAPHICS) override;
			virtual void wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd) override;
			virtual void submit(rhi::QueueType queueType = rhi::QueueType::GRAPHICS, bool waitAfterSubmitting = false) override;
			virtual bool present() override;
			virtual void wait_fences() override;

			virtual void copy_buffer(
				rhi::CommandBuffer* cmd,
				rhi::Buffer* srcBuffer,
				rhi::Buffer* dstBuffer,
				uint32_t size = 0,
				uint32_t srcOffset = 0,
				uint32_t dstOffset = 0) override;
			virtual void copy_texture(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Texture* dstTexture) override;
			virtual void blit_texture(
				rhi::CommandBuffer* cmd,
				rhi::Texture* srcTexture,
				rhi::Texture* dstTexture,
				const std::array<int32_t, 3>& srcOffset,
				const std::array<int32_t, 3>& dstOffset,
				uint32_t srcMipLevel = 0,
				uint32_t dstMipLevel = 0,
				uint32_t srcBaseLayer = 0,
				uint32_t dstBaseLayer = 0) override;
			virtual void copy_buffer_to_texture(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Texture* dstTexture) override;
			virtual void copy_texture_to_buffer(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Buffer* dstBuffer) override;
			virtual void set_viewports(rhi::CommandBuffer* cmd, std::vector<rhi::Viewport>& viewports) override;
			virtual void set_scissors(rhi::CommandBuffer* cmd, std::vector<rhi::Scissor>& scissors) override;
			virtual void push_constants(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline, void* data) override;
			virtual void bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) override;
			virtual void bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) override;
			virtual void bind_pipeline(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline) override;
			virtual void begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass, rhi::ClearValues& clearValues) override;
			virtual void end_render_pass(rhi::CommandBuffer* cmd) override;
			virtual void begin_rendering(rhi::CommandBuffer* cmd, rhi::RenderingBeginInfo* beginInfo) override;
			virtual void end_rendering(rhi::CommandBuffer* cmd) override;
			virtual void begin_rendering_swap_chain(rhi::CommandBuffer* cmd, rhi::ClearValues* clearValues) override;
			virtual void end_rendering_swap_chain(rhi::CommandBuffer* cmd) override;
			virtual void draw(rhi::CommandBuffer* cmd, uint64_t vertexCount) override;
			virtual void draw_indexed(
				rhi::CommandBuffer* cmd,
				uint32_t indexCount,
				uint32_t instanceCount,
				uint32_t firstIndex,
				int32_t vertexOffset,
				uint32_t firstInstance) override;
			virtual void draw_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) override;
			virtual void draw_indexed_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) override;
			virtual void dispatch(rhi::CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
			virtual void fill_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data) override;
			virtual void add_pipeline_barriers(rhi::CommandBuffer* cmd, const std::vector<rhi::PipelineBarrier>& barriers) override;

			virtual void wait_for_gpu() override;

			virtual void create_query_pool(rhi::QueryPool* queryPool, rhi::QueryPoolInfo* queryPoolInfo) override;
			virtual void create_query_pool(rhi::QueryPool* queryPool) override;
			virtual void begin_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex) override;
			virtual void end_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex) override;
			virtual void get_query_pool_result(
				const rhi::QueryPool* queryPool,
				std::vector<uint64_t>& outputData,
				uint32_t queryIndex,
				uint32_t queryCount,
				uint32_t stride) override;
			virtual void copy_query_pool_results(
				const rhi::CommandBuffer* cmd,
				const rhi::QueryPool* queryPool,
				uint32_t firstQuery,
				uint32_t queryCount,
				uint32_t stride,
				const rhi::Buffer* dstBuffer,
				uint32_t dstOffset = 0) override;
			virtual void reset_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex, uint32_t queryCount) override;
		
			virtual rhi::GPUMemoryUsage get_memory_usage() override;

			VulkanDevice* get_device() const { return _device.get(); }
			VulkanInstance* get_instance() const { return _instance.get(); }
			VulkanSwapChain* get_swap_chain() const { return _swapChain.get(); }
			uint32_t get_current_image_index() const { return _currentImageIndex; }
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			std::unique_ptr<VulkanInstance> _instance{ nullptr };
			std::unique_ptr<VulkanSurface> _surface{ nullptr };
			std::unique_ptr<VulkanDevice> _device{ nullptr };
			std::unique_ptr<VulkanCommandManager> _cmdManager{ nullptr };
			std::unique_ptr<VulkanSwapChain> _swapChain{ nullptr };
			std::unique_ptr<VulkanDescriptorManager> _descriptorManager{ nullptr };
			std::unique_ptr<VulkanPipelineLayoutCache> _pipelineLayoutCache{ nullptr };
			VulkanPipelineCache _pipelineCache;
			VulkanObjectPool _vkObjectPool;
			VulkanAttachmentManager _attachmentManager;

			uint32_t _currentImageIndex{ 0 };

			void set_swap_chain_image_barrier(rhi::CommandBuffer* cmd, bool useAfterDrawingImageBarrier);
			void recreate_swap_chain();
	};
}
