#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_command_manager.h"
#include "vulkan_swap_chain.h"
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include <VkBootstrap.h>

#include <vector>

namespace ad_astris::vulkan
{
	class VulkanRHI : public rhi::IEngineRHI
	{
		public:
			~VulkanRHI() final override = default;

			virtual void init(void* window) final override;
			virtual void cleanup() final override;

			virtual void create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info) final override;
			virtual void destroy_swap_chain(rhi::SwapChain* swapChain) final override;
		
			virtual void create_buffer(
				rhi::Buffer* buffer,
				rhi::BufferInfo* bufInfo,
				uint64_t size,
				void* data = nullptr) final override;

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

			virtual void begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType = rhi::GRAPHICS_QUEUE) final override;
			virtual void wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd) final override;
			virtual void submit(rhi::QueueType queueType = rhi::GRAPHICS_QUEUE) final override;

			virtual void bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer) final override;
			virtual void begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass) final override;
			virtual void end_render_pass(rhi::CommandBuffer* cmd) final override;

			// Only for tests. Will be removed in the final implementation
			VkInstance get_instance() { return _instance; }
			VulkanDevice get_device() { return _vulkanDevice; }
			VmaAllocator get_allocator() { return _allocator; }
			VkDebugUtilsMessengerEXT get_messenger() { return _debugMessenger; }
		
		private:
			VkInstance _instance{ VK_NULL_HANDLE };
			VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
			VmaAllocator _allocator;
			VulkanDevice _vulkanDevice;
			VulkanSwapChain* _swapChain{ nullptr };
			VulkanCommandManager* _cmdManager{ nullptr };

			vkb::Instance create_instance();
			void create_allocator();};
}
