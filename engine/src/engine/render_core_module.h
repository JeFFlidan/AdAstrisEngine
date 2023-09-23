#pragma once

#include "file_system/file_system.h"
#include "core/module.h"
#include "rhi/resources.h"
#include "rhi/engine_rhi.h"

#include <string>
#include <unordered_set>

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

	class IRenderPass
	{
		public:
			virtual ~IRenderPass() { }

			virtual std::string get_name() = 0;
		
			virtual uint32_t get_logical_pass_index() = 0;

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

	struct ShaderInputDesc
	{
		io::URI shaderPath;
		rhi::ShaderType type{ rhi::ShaderType::UNDEFINED };
		rhi::ShaderFormat format{ rhi::ShaderFormat::UNDEFINED };
		rhi::HLSLShaderModel minHlslShaderModel{ rhi::HLSLShaderModel::SM_6_0 };
		std::string entryPoint{ "main" };
		std::vector<std::string> defines;
		std::vector<std::string> includePaths;
	};

	struct ShaderOutputDesc
	{
		std::shared_ptr<void> internalBlob;
		const uint8_t* data{ nullptr };
		uint64_t dataSize;
		std::unordered_set<std::string> dependencies;
	};

	enum class ShaderCacheType
	{
		DXIL,
		SPIRV
	};

	struct ShaderCompilerInitContext
	{
		ModuleManager* moduleManager;
		io::FileSystem* fileSystem;
		ShaderCacheType cacheType;
	};

	class IShaderCompiler
	{
		public:
			virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) = 0;
	};

	struct ShaderManagerInitContext
	{
		ModuleManager* moduleManager;
		io::FileSystem* fileSystem;
		ShaderCacheType cacheType;
		rhi::IEngineRHI* rhi;
	};

	class IShaderManager
	{
		public:
			virtual ~IShaderManager() { }
		
			virtual void init(ShaderManagerInitContext& shaderManagerInitContext) = 0;
			virtual rhi::Shader* load_shader(
				const io::URI& relativeShaderPath,
				rhi::ShaderType shaderType,
				bool isEngineShader = true,
				rhi::HLSLShaderModel shaderModel = rhi::HLSLShaderModel::SM_6_0,
				const std::vector<std::string>& shaderDefines = {}) = 0;
			virtual rhi::Shader* get_shader(const io::URI& relativeShaderPath) = 0;
			virtual IShaderCompiler* get_shader_compiler() = 0;
	};

	struct RendererResourceManagerInitContext
	{
		rhi::IEngineRHI* rhi{ nullptr };
	};

	class IRendererResourceManager
	{
		public:
			virtual ~IRendererResourceManager() { }
		
			virtual void init(RendererResourceManagerInitContext& initContext) = 0;
			virtual void cleanup_staging_buffers() = 0;

			virtual void allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage) = 0;
			virtual void allocate_vertex_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual void allocate_index_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual void allocate_indirect_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual void allocate_storage_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& bufferName,
				uint64_t objectSizeInBytes,
				void* allObjects,
				uint64_t allObjectCount,
				uint64_t newObjectCount) = 0;

			virtual const rhi::Buffer* get_buffer(const std::string& bufferName) = 0;
	};
	
	class IRenderCoreModule : public IModule
	{
		public:
			virtual IRenderGraph* get_render_graph() = 0;
			virtual IShaderManager* get_shader_manager() = 0;
			virtual IRendererResourceManager* get_renderer_resource_manager() = 0;
	};
}

template<>
struct EnableBitMaskOperator<ad_astris::rcore::RenderGraphQueue>
{
	static const bool enable = true;
};
