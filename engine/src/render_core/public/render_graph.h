#pragma once

#include "rhi/engine_rhi.h"
#include "multithreading/task_types.h"

namespace ad_astris::rcore
{
	class ResourceDesc;
	class BufferDesc;
	class TextureDesc;
	class TextureGroupDesc;
	
	
	// Do I need this enum?
	enum class RenderGraphQueue
	{
		UNDEFINED = 0,
		GRAPHICS = 1 << 0,
		COMPUTE = 1 << 1,
		ASYNC_GRAPHICS = 1 << 2,
		ASYNC_COMPUTE = 1 << 3
	};

	class IRenderGraph;
	class IRendererResourceManager;
	
	class IRenderPassExecutor
	{
		public:
			virtual ~IRenderPassExecutor() { }
			// Must only be called before baking the render graph
			virtual void prepare_render_pass() = 0;
			virtual void execute(rhi::CommandBuffer* cmd) = 0;
	};

	// If rhi::TextureInfo* or rhi::BufferInfo* is nullptr, the resource is supposed to have been created
	// manually with IRendererResourceManager (or the resource has been added to this manager). In this case,
	// description name passed to the method must match the name of the resource in the IRendererResourceManager.
	// If the resource info is not nullptr, IRenderGraph will create the necessary resources with the description name
	class IRenderPass
	{
		public:
			virtual ~IRenderPass() { }

			virtual std::string get_name() = 0;
			virtual uint32_t get_logical_pass_index() = 0;
			virtual RenderGraphQueue get_queue() = 0;

			virtual void enable_multiview(uint32_t viewCount) = 0;
			virtual uint32_t get_view_count() = 0;

			virtual void set_executor(IRenderPassExecutor* renderPassExecutor) = 0;

			virtual TextureDesc* set_depth_stencil_input(const std::string& inputName) = 0;
			virtual TextureDesc* set_depth_stencil_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo = nullptr) = 0;
			virtual TextureDesc* add_color_input(const std::string& inputName) = 0;
			virtual TextureDesc* add_color_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo = nullptr) = 0;
			virtual TextureDesc* add_history_input(const std::string& inputName) = 0;

			virtual TextureDesc* add_texture_input(
				const std::string& inputName,
				rhi::TextureInfo* textureInfo = nullptr,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

			virtual TextureDesc* add_storage_texture_input(const std::string& inputName) = 0;
			virtual TextureDesc* add_storage_texture_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo = nullptr) = 0;

			virtual std::pair<TextureDesc*, TextureDesc*>& add_texture_blit_input_and_output(
				const std::string& inputName,
				const std::string& outputName,
				rhi::TextureInfo* outputTextureInfo = nullptr) = 0;

			virtual BufferDesc* add_uniform_buffer_input(
				const std::string& inputName,
				rhi::BufferInfo* bufferInfo = nullptr,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

			virtual BufferDesc* add_storage_buffer_read_only_input(
				const std::string& inputName,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;
			virtual BufferDesc* add_storage_buffer_read_write_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo = nullptr) = 0;

			virtual BufferDesc* add_transfer_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo = nullptr) = 0;

			virtual BufferDesc* add_vertex_buffer_input(const std::string& inputName) = 0;
			virtual BufferDesc* add_index_buffer_input(const std::string& inputName) = 0;
			virtual BufferDesc* add_indirect_buffer_input(const std::string& inputName) = 0;
	};

	// Render graph does not support transient attachments
	class IRenderGraph
	{
		public:
			virtual ~IRenderGraph() { }
		
			virtual void init(rhi::RHI* engineRHI) = 0;
			virtual void cleanup() = 0;
		
			virtual IRenderPass* add_new_pass(const std::string& passName, RenderGraphQueue queue) = 0;
			virtual IRenderPass* get_pass(const std::string& passName) = 0;
			virtual void set_swap_chain_input(const std::string& swapChainInputName) = 0;
			virtual void set_swap_chain_executor(IRenderPassExecutor* executor) = 0;

			virtual void bake() = 0;
			virtual void log() = 0;
		
			virtual TextureDesc* get_texture_desc(const std::string& textureName) = 0;
			virtual BufferDesc* get_buffer_desc(const std::string& bufferName) = 0;

			virtual void draw(tasks::TaskGroup* taskGroup) = 0;
	};
}

ENABLE_BIT_MASK(ad_astris::rcore::RenderGraphQueue)