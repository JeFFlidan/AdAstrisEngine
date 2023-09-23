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
		
			virtual void cleanup_staging_buffers() override;

			virtual void allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage) override;
			virtual void allocate_vertex_buffer(const std::string& bufferName, uint64_t size) override;
			virtual void allocate_index_buffer(const std::string& bufferName, uint64_t size) override;
			virtual void allocate_indirect_buffer(const std::string& bufferName, uint64_t size) override;
			virtual void allocate_storage_buffer(const std::string& bufferName, uint64_t size) override;
			virtual bool update_buffer(
				rhi::CommandBuffer* cmd,
				const std::string& bufferName,
				uint64_t objectSizeInBytes,
				void* allObjects,
				uint64_t allObjectCount,
				uint64_t newObjectCount) override;

			virtual const rhi::Buffer* get_buffer(const std::string& bufferName) override;

		private:
			rhi::IEngineRHI* _rhi{ nullptr };

			ThreadSafePoolAllocator<rhi::Buffer> _bufferPool;
			std::unordered_map<std::string, rhi::Buffer*> _bufferByItsName;
			std::vector<rhi::Buffer> _stagingBuffers;
			std::mutex _gpuBufferMutex;
			std::mutex _stagingBufferMutex;
			std::atomic_bool _isDeviceWaiting;
		
			void allocate_staging_buffer(rhi::Buffer& buffer, void* allObjects, uint64_t offset, uint64_t newObjectsSize);
			rhi::Buffer& get_new_staging_buffer();
	};
}
