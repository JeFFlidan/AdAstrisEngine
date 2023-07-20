#pragma once

#include "core/module.h"
#include "engine_core/material/shader_common.h"
#include "file_system/file_system.h"
#include "rhi/resources.h"
#include "rhi/engine_rhi.h"

#include <string>

namespace ad_astris::rcore
{
	class ResourceDesc;
	class BufferDesc;
	class TextureDesc;
	
	
	// Do I need this enum?
	enum class RenderGraphQueue
	{
		UNDEFINED = 0,
		GRAPHICS = 1 << 0,
		COMPUTE = 1 << 1,
		ASYNC_GRAPHICS = 1 << 2,
		ASYNC_COMPUTE = 1 << 3
	};

	class IShaderCompiler
	{
		public:
			virtual ~IShaderCompiler() = default;
			virtual void init(io::FileSystem* filsSystem) = 0;
		
			//Legacy
			virtual void compile_into_spv(io::URI& path, rhi::ShaderInfo* info) = 0;
		
			virtual void compile_shader_into_spv(ecore::shader::CompilationContext& compilationContext) = 0;
	};
	
	class IRenderPass
	{
		public:
			virtual ~IRenderPass() { }

			virtual std::string get_name() = 0;
		
			virtual uint32_t get_logical_pass_index() = 0;

			virtual uint32_t get_physical_pass_index() = 0;
			virtual void set_physical_pass_index(uint32_t newPhysicalIndex) = 0;

			virtual RenderGraphQueue get_queue() = 0;

			virtual void enable_multiview() = 0;
			virtual bool is_multiview_enabled() = 0;

			virtual TextureDesc* set_depth_stencil_input(const std::string& inputName) = 0;
			virtual TextureDesc* set_depth_stencil_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) = 0;
			virtual TextureDesc* add_attachment_input(const std::string& inputName) = 0;
			virtual TextureDesc* add_color_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) = 0;
			virtual TextureDesc* add_history_input(const std::string& inputName) = 0;

			virtual TextureDesc* add_texture_input(
				const std::string& inputName,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

			virtual TextureDesc* add_texture_input(
				const std::string& inputName,
				rhi::TextureInfo* textureInfo,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

			virtual TextureDesc* add_storage_texture_input(const std::string& inputName) = 0;
			virtual TextureDesc* add_storage_texture_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) = 0;

			virtual std::pair<TextureDesc*, TextureDesc*>& add_texture_blit_input_and_output(
				const std::string& inputName,
				const std::string& outputName,
				rhi::TextureInfo* outputTextureInfo) = 0;

			virtual BufferDesc* add_uniform_buffer_input(
				const std::string& inputName,
				rhi::BufferInfo* bufferInfo,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

			virtual BufferDesc* add_storage_buffer_read_only_input(
				const std::string& inputName,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;
			virtual BufferDesc* add_storage_buffer_read_write_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo) = 0;

			virtual BufferDesc* add_transfer_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo) = 0;

			virtual BufferDesc* add_vertex_buffer_input(const std::string& inputName) = 0;
			virtual BufferDesc* add_index_buffer_input(const std::string& inputName) = 0;
			virtual BufferDesc* add_indirect_buffer_input(const std::string& inputName) = 0;
	};

	// Render graph does not support transient attachments
	class IRenderGraph
	{
		public:
			virtual ~IRenderGraph() { }
		
			virtual void init(rhi::IEngineRHI* engineRHI) = 0;
			virtual void cleanup() = 0;
		
			virtual IRenderPass* add_new_pass(const std::string& passName, RenderGraphQueue queue) = 0;
			virtual IRenderPass* get_logical_pass(const std::string& passName) = 0;
			virtual rhi::RenderPass* get_physical_pass(const std::string& passName) = 0;
			virtual rhi::RenderPass* get_physical_pass(IRenderPass* logicalPass) = 0;
			virtual void create_swapchain(rhi::SwapChainInfo& swapChainInfo) = 0;
			virtual void set_swap_chain_source(const std::string& swapChainInputName) = 0;

			virtual void bake() = 0;
			virtual void log() = 0;
		
			virtual TextureDesc* get_texture_desc(const std::string& textureName) = 0;
			virtual BufferDesc* get_buffer_desc(const std::string& bufferName) = 0;

			virtual rhi::TextureView* get_physical_texture(TextureDesc* textureDesc) = 0;
			virtual rhi::TextureView* get_physical_texture(const std::string& textureName) = 0;
			virtual rhi::TextureView* get_physical_texture(uint32_t physicalIndex) = 0;
			virtual rhi::Buffer* get_physical_buffer(BufferDesc* bufferDesc) = 0;
			virtual rhi::Buffer* get_physical_buffer(const std::string& bufferName) = 0;
			virtual rhi::Buffer* get_physical_buffer(uint32_t physicalIndex) = 0;
	};
	
	class IRenderCoreModule : public IModule
	{
		public:
			virtual IShaderCompiler* get_shader_compiler() = 0;
			virtual IRenderGraph* get_render_graph() = 0;
	};
}

template<>
struct EnableBitMaskOperator<ad_astris::rcore::RenderGraphQueue>
{
	static const bool enable = true;
};