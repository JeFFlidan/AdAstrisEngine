#pragma once

#include "api.h"
#include "render_pass.h"
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
			RenderGraph(IRendererResourceManager* rendererResourceManager);
			RenderGraph(const RenderGraph&) = delete;
			RenderGraph(const RenderGraph&&) = delete;
			RenderGraph& operator=(const RenderGraph&) = delete;
			RenderGraph& operator=(const RenderGraph&&) = delete;
		
			virtual void init(rhi::RHI* engineRHI) override;
			virtual void cleanup() override;

			virtual IRenderPass* add_new_pass(const std::string& passName, RenderGraphQueue queue) override;
			virtual IRenderPass* get_pass(const std::string& passName) override;
		
			virtual void set_swap_chain_input(const std::string& swapChainInputName) override
			{
				_swapChainInputName = swapChainInputName;
			}

			virtual void set_swap_chain_executor(IRenderPassExecutor* executor) override
			{
				_swapChainExecutor = executor;
			}

			virtual void bake() override;
			virtual void log() override;

			virtual TextureDesc* get_texture_desc(const std::string& textureName) override;
			virtual BufferDesc* get_buffer_desc(const std::string& bufferName) override;

			virtual void draw(tasks::TaskGroup* taskGroup) override;

		private:
			rhi::RHI* _rhi{ nullptr };
			IRendererResourceManager* _rendererResourceManager{ nullptr };

			std::vector<std::unique_ptr<RenderPass>> _logicalPasses;
			std::unordered_map<std::string, uint16_t> _logicalPassIndexByItsName;

			std::vector<uint32_t> _sortedPasses;
			std::vector<std::unordered_set<uint32_t>> _passDependencies;
		
			std::vector<std::unique_ptr<ResourceDesc>> _logicalResources;
			std::unordered_map<std::string, uint32_t> _logicalResourceIndexByItsName;
		
			std::unordered_map<uint32_t, std::vector<rhi::PipelineBarrier>> _invalidatingPipelineBarriersByPassIndex;
			std::unordered_map<uint32_t, std::vector<rhi::PipelineBarrier>> _flushingPipelineBarriersByPassIndex;
			std::unordered_map<uint32_t, std::vector<rhi::PipelineBarrier>> _beforeBlitPipelineBarriersByPassIndex;
			std::unordered_map<uint32_t, std::vector<rhi::PipelineBarrier>> _afterBlitPipelineBarriersByPassIndex;

			std::unordered_map<uint32_t, rhi::RenderingBeginInfo> _renderingBeginInfoByPassIndex;

			std::string _swapChainInputName;
			IRenderPassExecutor* _swapChainExecutor{ nullptr };

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
			void build_rendering_begin_info();
			void build_barriers();
			
			void create_physical_texture(TextureDesc* logicalTexture);
			void create_physical_buffer(BufferDesc* logicalBuffer);

			bool check_if_compute(RenderPass* passHandle);
			bool check_if_graphics(RenderPass* passHandle);

			TextureDesc* get_texture_desc_handle(uint32_t index);
			TextureDesc* get_texture_desc_handle(ResourceDesc* resourceDesc);
			BufferDesc* get_buffer_desc_handle(uint32_t index);
			BufferDesc* get_buffer_desc_handle(ResourceDesc* resourceDesc);

			rhi::TextureView* get_physical_texture_view(const std::string& name);
			rhi::Texture* get_physical_texture(const std::string& name);
			rhi::Buffer* get_physical_buffer(const std::string& bufferName);

			void clear_collections();
	};
}