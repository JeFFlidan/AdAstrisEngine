#pragma once

#include "rhi/engine_rhi.h"
#include "core/pool_allocator.h"
#include "engine/render_core_module.h"
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace ad_astris::rcore::impl
{
	class RendererResourceManager : public IRendererResourceManager
	{
		public:
			virtual void init(RendererResourceManagerInitContext& initContext) override;
			virtual void cleanup() override;
		
			virtual void cleanup_staging_buffers() override;

			virtual rhi::Buffer* allocate_buffer(rhi::BufferInfo& bufferInfo) override;
			virtual rhi::Buffer* allocate_buffer(const std::string& bufferName, rhi::BufferInfo& bufferInfo) override;
			virtual rhi::Buffer* allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage) override;
			virtual rhi::Buffer* allocate_vertex_buffer(const std::string& bufferName, uint64_t size) override;
			virtual rhi::Buffer* allocate_index_buffer(const std::string& bufferName, uint64_t size) override;
			virtual rhi::Buffer* allocate_indirect_buffer(uint64_t size) override;
			virtual rhi::Buffer* allocate_indirect_buffer(const std::string& bufferName, uint64_t size) override;
			virtual rhi::Buffer* allocate_storage_buffer(uint64_t size);
			virtual rhi::Buffer* allocate_storage_buffer(const std::string& bufferName, uint64_t size) override;
			virtual void reallocate_buffer(rhi::Buffer* buffer, uint64_t newSize) override;
			virtual rhi::Buffer* reallocate_buffer(const std::string& bufferName, uint64_t newSize) override;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& bufferName,
				uint64_t objectSizeInBytes,
				void* allObjects,
				uint64_t allObjectCount,
				uint64_t newObjectCount) override;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd, 
				const std::string& srcBufferName,
				const std::string& dstBufferName,
				uint64_t objectSizeInBytes,
				uint64_t allObjectCount,
				uint64_t newObjectCount) override;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				rhi::Buffer* srcBuffer,
				rhi::Buffer* dstBuffer,
				uint64_t objectSizeInBytes,
				uint64_t allObjectCount,
				uint64_t newObjectCount) override;

			virtual rhi::Buffer* get_buffer(const std::string& bufferName) override;
			virtual void add_buffer(const std::string& bufferName, rhi::Buffer& buffer) override;
			virtual void bind_buffer_to_name(const std::string& bufferName, rhi::Buffer* buffer) override;

			virtual rhi::Texture* allocate_texture(const std::string& textureName, rhi::TextureInfo& textureInfo) override;
			virtual rhi::Texture* allocate_gpu_texture(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::Format format,
				rhi::ResourceUsage usage,
				rhi::ResourceFlags flags,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) override;
			virtual rhi::Texture* allocate_color_attachment(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) override;
			virtual rhi::Texture* allocate_depth_stencil_attachment(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) override;
			virtual rhi::Texture* allocate_cubemap(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::Format format,
				rhi::ResourceUsage usage,
				uint32_t mipLevels = 1,
				rhi::ResourceFlags flags = rhi::ResourceFlags::CUBE_TEXTURE) override;
			virtual rhi::Texture* allocate_custom_texture(
				const std::string& textureName,
				uint64_t width,
				uint64_t height,
				rhi::ResourceFlags flags = rhi::ResourceFlags::UNDEFINED,
				uint32_t mipLevels = 1,
				uint32_t layersCount = 1) override;
			virtual rhi::TextureView* allocate_texture_view(
				const std::string& textureViewName,
				const std::string& textureName,
				rhi::TextureViewInfo& info) override;
			virtual rhi::TextureView* allocate_texture_view(
				const std::string& textureViewName,
				const std::string& textureName,
				uint32_t baseMipLevel = 0,
				uint32_t baseLayer = 0,
				rhi::TextureAspect aspect = rhi::TextureAspect::UNDEFINED) override;

			virtual void update_2d_texture(rhi::CommandBuffer* cmd, const std::string& textureName, void* textureData, uint32_t width, uint32_t height) override;
			virtual void generate_mipmaps(rhi::CommandBuffer* cmd, const std::string& textureName) override;
			virtual void generate_mipmaps(rhi::CommandBuffer* cmd, rhi::Texture* texture) override;

			virtual rhi::Texture* get_texture(const std::string& textureName) override;
			virtual rhi::TextureView* get_texture_view(const std::string& textureViewName) override;
			virtual void add_texture(const std::string& textureName, rhi::Texture& texture) override;
			virtual void add_texture_view(const std::string& textureViewName, rhi::TextureView& textureView) override;


		private:
			rhi::IEngineRHI* _rhi{ nullptr };

			ThreadSafePoolAllocator<rhi::Buffer> _bufferPool;
			std::unordered_map<std::string, rhi::Buffer*> _bufferByItsName;
			std::vector<rhi::Buffer> _stagingBuffers;
			std::mutex _gpuBufferMutex;
			std::mutex _stagingBufferMutex;

			ThreadSafePoolAllocator<rhi::Texture> _texturePool;
			std::unordered_map<std::string, rhi::Texture*> _textureByItsName;
			std::mutex _textureMutex;

			ThreadSafePoolAllocator<rhi::TextureView> _textureViewPool;
			std::unordered_map<std::string, rhi::TextureView*> _textureViewByItsName;
			std::mutex _textureViewMutex;
		
			std::atomic_bool _isDeviceWaiting;
		
			rhi::Buffer* allocate_gpu_buffer(uint64_t size, rhi::ResourceUsage bufferUsage);
			void allocate_staging_buffer(rhi::Buffer& buffer, void* allObjects, uint64_t offset, uint64_t newObjectsSize);
			rhi::Buffer& get_new_staging_buffer();
			rhi::Buffer* check_buffer(const std::string& bufferName);
			rhi::Texture* check_texture(const std::string& textureName);
			rhi::TextureView* check_texture_view(const std::string& textureViewName);
	};
}
