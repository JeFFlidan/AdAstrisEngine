#pragma once

#include "rhi/engine_rhi.h"

namespace ad_astris::rcore
{
	struct RendererResourceManagerInitContext
	{
		rhi::RHI* rhi{ nullptr };
	};

	class IRendererResourceManager
	{
		public:
			virtual ~IRendererResourceManager() { }
		
			virtual void init(RendererResourceManagerInitContext& initContext) = 0;
			virtual void cleanup() = 0;
		
			virtual void cleanup_staging_buffers() = 0;

			virtual rhi::Buffer* allocate_buffer(rhi::BufferInfo& bufferInfo) = 0;
			virtual rhi::Buffer* allocate_buffer(const std::string& bufferName, rhi::BufferInfo& bufferInfo) = 0;
			virtual rhi::Buffer* allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage) = 0;
			virtual rhi::Buffer* allocate_vertex_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_index_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_indirect_buffer(uint64_t size) = 0;
			virtual rhi::Buffer* allocate_indirect_buffer(const std::string& bufferName, uint64_t size) = 0;
			virtual rhi::Buffer* allocate_storage_buffer(uint64_t size) = 0;
			virtual rhi::Buffer* allocate_storage_buffer(const std::string& bufferName, uint64_t size) = 0;
			// Implemented only for buffers with rhi::MemoryUsage::CPU
			virtual void reallocate_buffer(rhi::Buffer* buffer, uint64_t newSize) = 0;
			// Implemented only for buffers with rhi::MemoryUsage::CPU
			virtual rhi::Buffer* reallocate_buffer(const std::string& bufferName, uint64_t newSize) = 0;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& bufferName,
				uint64_t objectSizeInBytes,
				void* allObjects,
				uint64_t allObjectCount,
				uint64_t newObjectCount) = 0;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& srcBufferName,
				const std::string& dstBufferName,
				uint64_t objectSizeInBytes,
				uint64_t allObjectCount,
				uint64_t newObjectCount) = 0;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				rhi::Buffer* srcBuffer,
				rhi::Buffer* dstBuffer,
				uint64_t objectSizeInBytes,
				uint64_t allObjectCount,
				uint64_t newObjectCount) = 0;

			virtual rhi::Buffer* get_buffer(const std::string& bufferName) = 0;
			virtual void add_buffer(const std::string& bufferName, rhi::Buffer& buffer) = 0;
			virtual void bind_buffer_to_name(const std::string& bufferName, rhi::Buffer* buffer) = 0;

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
				uint32_t mipLevels = 0,
				uint32_t baseLayer = 0,
				uint32_t layerCount = 0,
				rhi::TextureAspect aspect = rhi::TextureAspect::UNDEFINED) = 0;

			virtual void update_2d_texture(rhi::CommandBuffer* cmd, const std::string& textureName, void* textureData, uint32_t width, uint32_t height) = 0;
			virtual void generate_mipmaps(rhi::CommandBuffer* cmd, const std::string& textureName) = 0;
			virtual void generate_mipmaps(rhi::CommandBuffer* cmd, rhi::Texture* texture) = 0;

			virtual rhi::Texture* get_texture(const std::string& textureName) = 0;
			virtual rhi::TextureView* get_texture_view(const std::string& textureViewName) = 0;
			virtual void add_texture(const std::string& textureName, rhi::Texture& texture) = 0;
			virtual void add_texture_view(const std::string& textureViewName, rhi::TextureView& textureView) = 0;
	};
}