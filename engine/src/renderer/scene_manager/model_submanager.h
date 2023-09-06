#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "resource_manager/resource_events.h"
#include "engine_core/fwd.h"

namespace ad_astris::renderer::impl
{
	class ModelSubmanager final : public SceneSubmanagerBase
	{
		public:
			ModelSubmanager(SceneSubmanagerInitializationContext& initContext);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void reset_temp_arrays() override;
			virtual void cleanup_staging_buffers() override;
			virtual bool need_allocation() override;

			rhi::Buffer* get_vertex_buffer_f32pntc()
			{
				return &_vertexBuffer_F32PNTC.buffer;
			}

			rhi::Buffer* get_index_buffer_f32pntc()
			{
				return &_indexBuffer_F32PNTC.buffer;
			}

			rhi::Buffer* get_output_plane_vertex_buffer()
			{
				return &_outputPlaneVertexBuffer_F32PC;
			}

		private:
			GPUBuffer _vertexBuffer_F32PNTC;
			GPUBuffer _indexBuffer_F32PNTC;
			rhi::Buffer _outputPlaneVertexBuffer_F32PC;
			bool _areDefaultBuffersAllocated{ false };
			bool _areGPUBuffersAllocated{ false };

			std::vector<uint8_t> vertexArray_F32PNTC;		// Contains all vertices of all models with vertex format Float32 Position Normal Tangent TexCoord
			std::vector<uint8_t> indexArray_F32PNTC;

			struct
			{
				rhi::Buffer vertexBuffer_F32PNTC;		// CPU buffer that contains all vertices of all models or all vertices of recently loaded models. It depends on occupancy of gpu buffer. Always destroyed after submitting _transferCmdBuffer
				rhi::Buffer indexBuffer_F32PNTC;
				rhi::Buffer outputPlaneVertexBuffer_F32PC;
			} _stagingBuffers;

			std::vector<rhi::Buffer*> _stagingBuffersToDelete;

			std::vector<ecore::StaticModelHandle> _loadedStaticModelHandles;
			uint64_t _loadedModelsVertexArraySize_F32PNTC{ 0 };
			uint64_t _loadedModelsIndexArraySize_F32PNTC{ 0 };

			virtual void subscribe_to_events() override;
			void allocate_staging_buffers(rhi::CommandBuffer& cmdBuffer);
			void allocate_gpu_buffers(rhi::CommandBuffer& cmdBuffer);
			void allocate_default_buffers(rhi::CommandBuffer& cmdBuffer);
	};

	IMPLEMENT_SUBMANAGER_EVENT(ModelSubmanager)
}