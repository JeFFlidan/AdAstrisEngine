#pragma once

#include "api.h"
#include "render_pass.h"
#include "engine/render_core_module.h"
#include "rhi/engine_rhi.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace ad_astris::rcore::impl
{
	/* 
	 * 1. Think about aliasing attachments. If I want to implement ray tracing and something like Nanite from UE,
	 * it may become important to save memory
	 * 2. Think about transient attachments. It's important to save memory, however I don't know if D3D12 supports this capability
	 */
	class RENDER_CORE_API RenderGraph : public IRenderGraph
	{
		public:
			RenderGraph(rhi::IEngineRHI* engineRHI);

			virtual void cleanup() override;

			RenderGraph(const RenderGraph&) = delete;
			RenderGraph(const RenderGraph&&) = delete;
			RenderGraph& operator=(const RenderGraph&) = delete;
			RenderGraph& operator=(const RenderGraph&&) = delete;

			virtual IRenderPass* add_new_pass(const std::string& passName, RenderGraphQueue queue) override;
			virtual IRenderPass* get_logical_pass(const std::string& passName) override;
			virtual rhi::RenderPass* get_physical_pass(const std::string& passName) override;
			virtual rhi::RenderPass* get_physical_pass(IRenderPass* logicalPass) override;
		
			virtual void create_swapchain(rhi::SwapChainInfo& swapChainInfo) override
			{
				if (!_swapChain)
					_engineRHI->create_swap_chain(_swapChain, &swapChainInfo);
			}
		
			virtual void set_swap_chain_source(const std::string& swapChainInputName) override
			{
				_swapChainInputName = swapChainInputName;
			}

			virtual void bake() override;
			virtual void log() override;

			virtual TextureDesc* get_texture_desc(const std::string& textureName) override;
			virtual BufferDesc* get_buffer_desc(const std::string& bufferName) override;
		
			virtual rhi::TextureView* get_physical_texture(TextureDesc* textureDesc) override;
			virtual rhi::TextureView* get_physical_texture(const std::string& textureName) override;
			virtual rhi::TextureView* get_physical_texture(uint32_t physicalIndex) override;
			virtual rhi::Buffer* get_physical_buffer(BufferDesc* bufferDesc) override;
			virtual rhi::Buffer* get_physical_buffer(const std::string& bufferName) override;
			virtual rhi::Buffer* get_physical_buffer(uint32_t physicalIndex) override;

		private:
			rhi::IEngineRHI* _engineRHI{ nullptr };

			std::vector<std::unique_ptr<RenderPass>> _logicalPasses;
			std::unordered_map<std::string, uint16_t> _logicalPassIndexByItsName;

			std::vector<uint32_t> _sortedPasses;
			std::vector<std::unordered_set<uint32_t>> _passDependencies;
		
			std::vector<std::unique_ptr<ResourceDesc>> _logicalResources;
			std::unordered_map<std::string, uint32_t> _logicalResourceIndexByItsName;

			std::vector<rhi::Texture> _physicalTextures;
			std::vector<rhi::TextureView> _physicalTextureViews;	// Do not forget to clear texture and texture view
			std::vector<rhi::Buffer> _physicalBuffers;

			std::vector<rhi::RenderPass> _physicalPasses;

			std::unordered_set<uint32_t> _passToIgnore;
			std::unordered_map<uint32_t, std::vector<rhi::PipelineBarrier>> _barriersByResourceName;
			std::vector<std::vector<std::pair<uint32_t, uint32_t>>> _passBarriers;

			std::string _swapChainInputName;
			rhi::SwapChain* _swapChain{ nullptr };

			void solve_graph(RenderPass* passHandle, uint32_t passesInStackCount);
			void parse_passes_recursively(
				RenderPass* currentPass,
				std::unordered_set<uint32_t>& writtenPasses,
				uint32_t passesInStackCount,
				bool checkIfEmpty,
				bool dontSkipIfSelfDependency);
			void filter_pass_order();
			void optimize_pass_order();
		
			void build_physical_resources();
			void build_physical_passes();
			void build_barriers();
			
			void create_physical_texture(TextureDesc* logicalTexture);
			void create_physical_buffer(BufferDesc* logicalBuffer);
			void setup_physical_pass_queue(rhi::RenderPassInfo& physPassInfo, RenderPass* logicalPass);
			void setup_depth_stencil_render_target(rhi::RenderTarget& renderTarget, rhi::TextureView& textureView);
			void setup_color_render_target(rhi::RenderTarget& renderTarget, rhi::TextureView& textureView);

			void setup_barrier(
				RenderPass* passHandle,
				ResourceDesc* resourceDesc,
				rhi::ResourceLayout srcLayout,
				rhi::ResourceLayout dstLayout);

			void setup_incomplete_barrier(
				ResourceDesc* resourceDesc,
				rhi::ResourceLayout srcLayout,
				std::unordered_map<uint32_t, uint32_t>& incompleteBarrierByResIndex);

			bool finish_incomplete_barrier(
				RenderPass* passHandle,
				ResourceDesc* resourceDesc,
				rhi::ResourceLayout dstLayout,
				std::unordered_map<uint32_t, uint32_t>& incompleteBarrierByResIndex);

			bool check_if_compute(RenderPass* passHandle);
			bool check_if_graphics(RenderPass* passHandle);

			TextureDesc* get_texture_desc_handle(uint32_t index);
			TextureDesc* get_texture_desc_handle(ResourceDesc* resourceDesc);
			BufferDesc* get_buffer_desc_handle(uint32_t index);
			BufferDesc* get_buffer_desc_handle(ResourceDesc* resourceDesc);
	};
}