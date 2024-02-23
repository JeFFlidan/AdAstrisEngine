#pragma once

#include "common.h"
#include "culling_submanager.h"
#include "material_submanager.h"
#include "resource_manager/resource_events.h"
#include "shader_interop_renderer.h"

namespace ad_astris::renderer::impl
{
	class ModelSubmanager
	{
		public:
			ModelSubmanager(MaterialSubmanager* materialSubmanager, CullingSubmanager* indirectDrawingSubmanager);

			void update(rhi::CommandBuffer& cmdBuffer);
			void cleanup_after_update();
			bool need_update();

			rhi::Buffer* get_vertex_buffer_f32pntc()
			{
				return RENDERER_RESOURCE_MANAGER()->get_buffer(VERTEX_BUFFER_F32PNTC_NAME);
			}

			rhi::Buffer* get_index_buffer_f32pntc()
			{
				return RENDERER_RESOURCE_MANAGER()->get_buffer(INDEX_BUFFER_F32PNTC_NAME);
			}

			rhi::Buffer* get_output_plane_vertex_buffer()
			{
				return RENDERER_RESOURCE_MANAGER()->get_buffer(OUTPUT_PLANE_VERTEX_BUFFER_NAME);
			}

			rhi::Buffer* get_model_instance_buffer()
			{
				return RENDERER_RESOURCE_MANAGER()->get_buffer(MODEL_INSTANCE_BUFFER_NAME);
			}

			void add_model(ecs::Entity entity);

		private:
			const std::string VERTEX_BUFFER_F32PNTC_NAME = "vertex_buffer_f32pntc";
			const std::string INDEX_BUFFER_F32PNTC_NAME = "index_buffer_f32pntc";
			const std::string OUTPUT_PLANE_VERTEX_BUFFER_NAME = "output_plane_buffer";
			const std::string MODEL_INSTANCE_BUFFER_NAME = "model_instance_buffer";

			MaterialSubmanager* _materialSubmanager;
		
			bool _areGPUBuffersAllocated{ false };
			std::unordered_map<UUID, std::vector<ecs::Entity>> _entitiesByModelUUID;

			std::vector<uint8_t> _vertexArray_F32PNTC;		// Contains all vertices of all models with vertex format Float32 Position Normal Tangent TexCoord
			std::vector<uint8_t> _indexArray_F32PNTC;

			std::vector<ecs::Entity> _createdModels;
			uint64_t _loadedModelsVertexArraySize_F32PNTC{ 0 };
			uint64_t _loadedModelsIndexArraySize_F32PNTC{ 0 };
		
			std::unordered_set<ecs::Entity> _staticModelEntities;
			std::unordered_set<ecs::Entity> _skeletalModelEntities;
			std::vector<RendererModelInstance> _modelInstances;

			CullingSubmanager* _indirectDrawingSubmanager{ nullptr };

			void subscribe_to_events();
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
			void allocate_gpu_buffers(rhi::CommandBuffer& cmd);
			void update_cpu_arrays(rhi::CommandBuffer& cmd);
			resource::ResourceAccessor<ecore::StaticModel> get_static_model_handle(ecs::Entity entity);
			bool is_model_loaded(UUID modelUUID);
	};
}
