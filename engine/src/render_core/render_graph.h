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
	class RENDER_CORE_API RenderGraph : public IRenderGraph
	{
		public:
			RenderGraph(rhi::IEngineRHI* engineRHI);

			RenderGraph(const RenderGraph&) = delete;
			RenderGraph(const RenderGraph&&) = delete;
			RenderGraph& operator=(const RenderGraph&) = delete;
			RenderGraph& operator=(const RenderGraph&&) = delete;

			virtual IRenderPass* add_new_pass(const std::string& passName, RenderGraphQueue queue) override;
			virtual IRenderPass* get_pass(const std::string& passName) override;

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
			std::unordered_map<std::string, uint16_t> _logicalPassNameToIndex;

			std::vector<std::unique_ptr<ResourceDesc>> _logicalResources;
			std::unordered_map<std::string, uint32_t> _logicalResourceNameToIndex;
		
			std::vector<rhi::TextureView*> _physicalTextureViews;
			std::vector<rhi::Buffer*> _physicalBuffers;

			TextureDesc* get_texture_desc_handle(uint32_t index);
			TextureDesc* get_texture_desc_handle(ResourceDesc* resourceDesc);
			BufferDesc* get_buffer_desc_handle(uint32_t index);
			BufferDesc* get_buffer_desc_handle(ResourceDesc* resourceDesc);
	};
}