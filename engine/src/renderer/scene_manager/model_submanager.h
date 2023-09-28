#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "resource_manager/resource_events.h"
#include "engine_core/fwd.h"
#include "shader_interop_renderer.h"
#include "material_submanager.h"

namespace ad_astris::renderer::impl
{
	class ModelSubmanager final : public SceneSubmanagerBase
	{
		public:
			ModelSubmanager(SceneSubmanagerInitializationContext& initContext, MaterialSubmanager* materialSubmanager);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_after_update() override;
			virtual bool need_update() override;

			rhi::Buffer* get_vertex_buffer_f32pntc()
			{
				return _rendererResourceManager->get_buffer(VERTEX_BUFFER_F32PNTC_NAME);
			}

			rhi::Buffer* get_index_buffer_f32pntc()
			{
				return _rendererResourceManager->get_buffer(INDEX_BUFFER_F32PNTC_NAME);
			}

			rhi::Buffer* get_output_plane_vertex_buffer()
			{
				return _rendererResourceManager->get_buffer(OUTPUT_PLANE_VERTEX_BUFFER_NAME);
			}

			rhi::Buffer* get_model_instance_buffer()
			{
				return _rendererResourceManager->get_buffer(MODEL_INSTANCE_BUFFER_NAME);
			}

			void add_static_model(ecore::StaticModelHandle handle, ecs::Entity& entity);

		private:
			const std::string VERTEX_BUFFER_F32PNTC_NAME = "vertex_buffer_f32pntc";
			const std::string INDEX_BUFFER_F32PNTC_NAME = "index_buffer_f32pntc";
			const std::string OUTPUT_PLANE_VERTEX_BUFFER_NAME = "output_plane_buffer";
			const std::string MODEL_INSTANCE_BUFFER_NAME = "model_instance_buffer";

			MaterialSubmanager* _materialSubmanager;
		
			bool _areGPUBuffersAllocated{ false };

			std::vector<uint8_t> _vertexArray_F32PNTC;		// Contains all vertices of all models with vertex format Float32 Position Normal Tangent TexCoord
			std::vector<uint8_t> _indexArray_F32PNTC;

			std::vector<ecore::StaticModelHandle> _loadedStaticModelHandles;
			uint64_t _loadedModelsVertexArraySize_F32PNTC{ 0 };
			uint64_t _loadedModelsIndexArraySize_F32PNTC{ 0 };
			
			std::unordered_set<ecs::Entity> _staticModelEntities;
			std::unordered_set<ecs::Entity> _skeletalModelEntities;
			std::vector<RendererModelInstance> _modelInstances;

			virtual void subscribe_to_events() override;
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
			void allocate_gpu_buffers(rhi::CommandBuffer& cmd);
			void update_cpu_arrays();
	};
}
