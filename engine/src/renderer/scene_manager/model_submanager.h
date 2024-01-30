#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "resource_manager/resource_events.h"
#include "engine_core/fwd.h"
#include "shader_interop_renderer.h"
#include "material_submanager.h"

namespace ad_astris::renderer::impl
{
	class ModelSubmanager;
	
	class IndirectBufferDesc
	{
		public:
			IndirectBufferDesc(ModelSubmanager* modelSubmanager, uint32_t indirectBufferIndex);
			void cleanup_after_update();

			// Returns true if model hasn't been added to the indirect command buffer and new batch is created
			bool add_model(
				UUID cpuModelUUID,
				uint32_t vertexOffset,
				uint32_t indexOffset,
				ecore::StaticModelData& modelData);
			void add_instance(uint32_t objectIndex, UUID modelUUID);
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
			rhi::Buffer* get_indirect_buffer() { return _indirectBuffer; }
			rhi::Buffer* get_culling_instance_indices_buffer() { return _cullingInstanceIndicesBuffer; }
			rhi::Buffer* get_model_instance_id_buffer() { return _modelInstanceIDBuffer; }
			uint32_t get_all_instance_count() { return _instanceCount; }
			uint32_t get_indirect_command_count() { return _indirectCommands.size(); }
			uint32_t get_batch_instance_count(uint32_t batchIndex) { return _indirectCommands[batchIndex].instanceCount; }
			std::string get_indirect_buffer_name() { return get_name_with_index("indirect_buffer"); }

			std::vector<UUID>& get_uuids() { return _uuidInRightOrder; }	// TEMP
			void add_renderer_model_instance_id(uint32_t id)
			{
				RendererModelInstanceID instanceId;
				instanceId.id = id;
				_modelInstanceIDs.emplace_back(instanceId);
			}

		private:
			ModelSubmanager* _modelSubmanager;

			std::unordered_map<UUID, uint32_t> _indirectBatchIndexByCPUModelUUID;
			std::vector<UUID> _uuidInRightOrder;		// TEMP
			std::vector<DrawIndexedIndirectCommand> _indirectCommands;
			std::vector<CullingInstanceIndices> _cullingInstanceIndices;
			std::vector<RendererModelInstanceID> _modelInstanceIDs;
			uint32_t _instanceCount{ 0 };
			rhi::Buffer* _indirectBuffer;
			rhi::Buffer* _cullingInstanceIndicesBuffer;
			rhi::Buffer* _modelInstanceIDBuffer;
			uint32_t _indirectBufferIndex{ 0 };

			void allocate_gpu_buffers();
			std::string get_name_with_index(const std::string& name);
	};
	
	class ModelSubmanager final : public SceneSubmanagerBase
	{
		public:
			ModelSubmanager(MaterialSubmanager* materialSubmanager);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_after_update() override;
			virtual bool need_update() override;

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
		
			IndirectBufferDesc* get_indirect_buffer_desc() { return _indirectBufferDesc.get(); }

			void add_static_model(ecore::StaticModelHandle handle, ecs::Entity& entity);

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
		
			std::vector<ecore::StaticModelHandle> _loadedStaticModelHandles;
			uint64_t _loadedModelsVertexArraySize_F32PNTC{ 0 };
			uint64_t _loadedModelsIndexArraySize_F32PNTC{ 0 };
		
			std::unordered_set<ecs::Entity> _staticModelEntities;
			std::unordered_set<ecs::Entity> _skeletalModelEntities;
			std::vector<RendererModelInstance> _modelInstances;

			std::unique_ptr<IndirectBufferDesc> _indirectBufferDesc;

			virtual void subscribe_to_events() override;
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
			void allocate_gpu_buffers(rhi::CommandBuffer& cmd);
			void update_cpu_arrays(rhi::CommandBuffer& cmd);

	};
}
