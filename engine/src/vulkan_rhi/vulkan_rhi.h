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
#include "vulkan_descriptor_manager.h"
#include "vulkan_pipeline_layout_cache.h"
#include "core/pool_allocator.h"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vkbootstrap/VkBootstrap.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanRHI : public rhi::IEngineRHI
	{
		public:
			// TODO Create custom destructor
			~VulkanRHI() final override = default;

			virtual void init(rhi::RHIInitContext& initContext) final override;
			virtual void cleanup() final override;

			virtual void create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info) final override;
			virtual void destroy_swap_chain(rhi::SwapChain* swapChain) final override;
			virtual void get_swap_chain_texture_views(std::vector<rhi::TextureView>& textureViews) final override;

			virtual bool acquire_next_image(uint32_t& nextImageIndex, uint32_t currentFrameIndex) final override;
		
			virtual void create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data = nullptr) final override;
			virtual void create_buffer(rhi::Buffer* buffer, void* data = nullptr) final override;
			virtual void destroy_buffer(rhi::Buffer* buffer) final override;

			virtual void update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data) final override;
			virtual void create_texture(rhi::Texture* texture, rhi::TextureInfo* texInfo) final override;
			virtual void create_texture(rhi::Texture* texture) final override;
			virtual void create_texture_view(
				rhi::TextureView* textureView,
				rhi::TextureViewInfo* viewInfo,
				rhi::Texture* texture) final override;
			virtual void create_texture_view(rhi::TextureView* textureView, rhi::Texture* texture) final override;
			virtual void create_sampler(rhi::Sampler* sampler, rhi::SamplerInfo* sampInfo) final override;
			virtual void create_shader(rhi::Shader* shader, rhi::ShaderInfo* shaderInfo) final override;
			virtual void create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo) final override;
			virtual void create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info) final override;
			virtual void create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info) final override;

			virtual uint32_t get_descriptor_index(rhi::Buffer* buffer) final override;
			virtual uint32_t get_descriptor_index(rhi::TextureView* textureView) final override;
			virtual uint32_t get_descriptor_index(rhi::Sampler* sampler) final override;
			virtual void bind_uniform_buffer(rhi::Buffer* buffer, uint32_t slot, uint32_t size, uint32_t offset) override;

			virtual void begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType = rhi::QueueType::GRAPHICS) final override;
			virtual void wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd) final override;
			virtual void submit(rhi::QueueType queueType = rhi::QueueType::GRAPHICS, bool waitAfterSubmitting = false) final override;
			virtual bool present() final override;
			virtual void wait_fences() final override;

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
			virtual void set_viewports(rhi::CommandBuffer* cmd, std::vector<rhi::Viewport>& viewports) final override;
			virtual void set_scissors(rhi::CommandBuffer* cmd, std::vector<rhi::Scissor>& scissors) final override;
			virtual void bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void bind_pipeline(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline) final override;
			virtual void begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass, rhi::ClearValues& clearValues) final override;
			virtual void end_render_pass(rhi::CommandBuffer* cmd) final override;
			virtual void begin_rendering(rhi::CommandBuffer* cmd, rhi::RenderingBeginInfo* beginInfo) override;
			virtual void end_rendering(rhi::CommandBuffer* cmd) override;
			virtual void begin_rendering_swap_chain(rhi::CommandBuffer* cmd, rhi::ClearValues* clearValues) override;
			virtual void end_rendering_swap_chain(rhi::CommandBuffer* cmd) override;
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

			virtual void wait_for_gpu() override;
			virtual uint32_t get_buffer_count() override { return _swapChain->get_buffers_count(); }

			VulkanDevice* get_device() { return _device.get(); }
			VkInstance get_instance() { return _instance; }
			VulkanSwapChain* get_swap_chain() { return _swapChain.get(); }
			uint32_t get_current_image_index() { return _currentImageIndex; }
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			VkInstance _instance{ VK_NULL_HANDLE };
			VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
			VmaAllocator _allocator;
			std::unique_ptr<VulkanDevice> _device{ nullptr };
			std::unique_ptr<VulkanCommandManager> _cmdManager{ nullptr };
			std::unique_ptr<VulkanSwapChain> _swapChain{ nullptr };
			std::unique_ptr<VulkanDescriptorManager> _descriptorManager{ nullptr };
			std::unique_ptr<VulkanPipelineLayoutCache> _pipelineLayoutCache{ nullptr };
			VkPipelineCache _pipelineCache;

			std::vector<std::unique_ptr<VulkanPipeline>> _vulkanPipelines;
			std::vector<std::unique_ptr<VulkanShader>> _vulkanShaders;
			std::vector<std::unique_ptr<VulkanRenderPass>> _vulkanRenderPasses;
			std::vector<std::unique_ptr<VulkanSampler>> _vulkanSamplers;
			std::vector<std::unique_ptr<VulkanTextureView>> _vulkanTextureViews;
			std::vector<std::unique_ptr<VulkanTexture>> _vulkanTextures;
			std::vector<std::unique_ptr<VulkanBuffer>> _vulkanBuffers;

			struct AttachmentDesc
			{
				VkImageCreateInfo imageCreateInfo;
				struct ViewDesc
				{
					uint32_t viewArrayIndex;
					VkImageViewCreateInfo imageViewCreateInfo;
				};
				std::vector<ViewDesc> viewDescriptions;
			};
			ThreadSafePoolAllocator<AttachmentDesc> _attachmentDescPool;
			std::unordered_map<VulkanTexture*, AttachmentDesc*> _viewIndicesByTexturePtr;

			uint32_t _currentImageIndex{ 0 };

			vkb::Instance create_instance();
			void create_allocator();
			void create_pipeline_cache();
			void save_pipeline_cache();

			void set_swap_chain_image_barrier(rhi::CommandBuffer* cmd, bool useAfterDrawingImageBarrier);
			void recreate_swap_chain();
	};
}
