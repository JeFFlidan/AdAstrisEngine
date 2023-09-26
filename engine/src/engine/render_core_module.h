#pragma once

#include "file_system/file_system.h"
#include "core/module.h"
#include "rhi/resources.h"
#include "rhi/engine_rhi.h"
#include "multithreading/task_composer.h"

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

	class IRenderGraph;
	class IRendererResourceManager;
	
	class IRenderPassExecutor
	{
		public:
			virtual ~IRenderPassExecutor() { }
			// Must only be called before baking the render graph
			virtual void prepare_render_pass(IRenderGraph* renderGraph, IRendererResourceManager* rendererResourceManager) = 0;
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
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) = 0;

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
		
			virtual void init(rhi::IEngineRHI* engineRHI) = 0;
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
			virtual void cleanup() = 0;
		
			virtual void cleanup_staging_buffers() = 0;

			virtual rhi::Buffer* allocate_buffer(const std::string& bufferName, rhi::BufferInfo& bufferInfo) = 0;
			virtual rhi::Buffer* allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage) = 0;
			virtual rhi::Buffer* allocate_vertex_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_index_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_indirect_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_storage_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& bufferName,
				uint64_t objectSizeInBytes,
				void* allObjects,
				uint64_t allObjectCount,
				uint64_t newObjectCount) = 0;
		
			virtual rhi::Buffer* get_buffer(const std::string& bufferName) = 0;
			virtual void add_buffer(const std::string& bufferName, rhi::Buffer& buffer) = 0;

			virtual rhi::Texture* allocate_texture(const std::string& textureName, rhi::TextureInfo& textureInfo) = 0;
			virtual rhi::Texture* allocate_gpu_texture(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::Format format,
				rhi::ResourceUsage usage,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) = 0;
			virtual rhi::Texture* allocate_color_attachment(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) = 0;
			virtual rhi::Texture* allocate_depth_stencil_attachment(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) = 0;
			virtual rhi::Texture* allocate_cubemap(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::Format format,
				rhi::ResourceUsage usage,
				uint32_t mipLevels = 1,
				rhi::ResourceFlags flags = rhi::ResourceFlags::CUBE_TEXTURE) = 0;
			virtual rhi::Texture* allocate_custom_texture(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) = 0;
			virtual rhi::TextureView* allocate_texture_view(
				const std::string& textureViewName,
				const std::string& textureName,
				rhi::TextureViewInfo& info) = 0;
			virtual rhi::TextureView* allocate_texture_view(
				const std::string& textureViewName,
				const std::string& textureName,
				uint32_t baseMipLevel = 0,
				uint32_t baseLayer = 0,
				rhi::TextureAspect aspect = rhi::TextureAspect::UNDEFINED) = 0;

			virtual rhi::Texture* get_texture(const std::string& textureName) = 0;
			virtual rhi::TextureView* get_texture_view(const std::string& textureViewName) = 0;
			virtual void add_texture(const std::string& textureName, rhi::Texture& texture) = 0;
			virtual void add_texture_view(const std::string& textureViewName, rhi::TextureView& textureView) = 0;
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

namespace ad_astris::rcore
{
	class ResourceDesc
	{
		public:
			enum { Unused = ~0u };
		
			enum class Type
			{
				BUFFER,
				TEXTURE,
				UNDEFINED
			};

			ResourceDesc(Type type, const std::string& name, uint32_t logicalIndex)
				: _type(type), _name(name), _logicalIndex(logicalIndex) { }
	
			Type get_type()
			{
				return _type;
			}	

			std::string get_name()
			{
				return _name;
			}
		
			void set_name(const std::string& newName)
			{
				_name = newName;
			}

			uint32_t get_logical_index()
			{
				return _logicalIndex;
			}

			void set_logical_index(uint32_t newLogicalIndex)
			{
				_logicalIndex = newLogicalIndex;
			}

			std::unordered_set<uint32_t>& get_written_in_passes()
			{
				return _writtenInPasses;
			}
		
			void add_written_in_pass(uint8_t passIndex)
			{
				_writtenInPasses.insert(passIndex);
			}

			std::unordered_set<uint32_t>& get_read_in_passes()
			{
				return _readInPasses;
			}
		
			void add_read_in_pass(uint8_t passIndex)
			{
				_readInPasses.insert(passIndex);
			}

			RenderGraphQueue get_queues()
			{
				return _queues;
			}
		
			void add_queue(RenderGraphQueue queue)
			{
				_queues |= queue;
			}

			rhi::ShaderType get_shader_stages()
			{
				return _shaderStages;
			}

			void add_shader_stage(rhi::ShaderType stage)
			{
				_shaderStages |= stage;
			}
		
		private:
			Type _type = Type::UNDEFINED;
			std::string _name;
			uint32_t _logicalIndex = Unused;
			std::unordered_set<uint32_t> _writtenInPasses;
			std::unordered_set<uint32_t> _readInPasses;
			RenderGraphQueue _queues{ RenderGraphQueue::UNDEFINED };		// Do I need this?
			rhi::ShaderType _shaderStages{ rhi::ShaderType::UNDEFINED };
	};

	class BufferDesc : public ResourceDesc
	{
		public:
			BufferDesc(const std::string& name, uint32_t logicalIndex)
				: ResourceDesc(Type::BUFFER, name, logicalIndex) { }

			/** Sets new BufferInfo. Old buffer usage will be added to the new BufferInfo
			 * @param bufferInfo should be valid pointer to the BufferInfo object.
			 */
			void set_buffer_info(rhi::BufferInfo* bufferInfo)
			{
				if (bufferInfo)
				{
					rhi::ResourceUsage oldUsage = _info.bufferUsage;
					_info = *bufferInfo;
					_info.bufferUsage |= oldUsage;
					_hasInfo = true;
				}
			}

			bool has_buffer_info()
			{
				return _hasInfo;
			}
		
			rhi::BufferInfo& get_buffer_info()
			{
				return _info;
			}

			void add_buffer_usage(rhi::ResourceUsage usage)
			{
				_info.bufferUsage |= usage;
			}
		
			rhi::ResourceUsage get_buffer_usage()
			{
				return _info.bufferUsage;
			}
		
		private:
			rhi::BufferInfo _info;
			bool _hasInfo{ false };
			rhi::ShaderType _shaderStages{ rhi::ShaderType::UNDEFINED };
	};

	class TextureDesc : public ResourceDesc
	{
		public:
			TextureDesc(const std::string& name, uint32_t logicalIndex)
				: ResourceDesc(Type::TEXTURE, name, logicalIndex) { }

			/** Sets new TextureInfo. Old texture usage will be added to the new TextureInfo
			* @param textureInfo should be valid pointer to the TextureInfo object.
			*/
			void set_texture_info(rhi::TextureInfo* textureInfo)
			{
				if (textureInfo)
				{
					rhi::ResourceUsage oldUsage = _info.textureUsage;
					_info = *textureInfo;
					_info.textureUsage |= oldUsage;
					_hasInfo = true;
				}
			}

			bool has_texture_info()
			{
				return _hasInfo;
			}
		
			rhi::TextureInfo& get_texture_info()
			{
				return _info;
			}

			void add_texture_usage(rhi::ResourceUsage usage)
			{
				_info.textureUsage |= usage;
			}
		
			rhi::ResourceUsage get_texture_usage()
			{
				return _info.textureUsage;
			}

		private:
			rhi::TextureInfo _info;
			bool _hasInfo{ false };
	};
}
