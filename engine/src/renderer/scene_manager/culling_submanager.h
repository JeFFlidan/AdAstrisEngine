#pragma once

#include "common.h"
#include "shader_interop_renderer.h"
#include "engine_core/basic_components.h"
#include "engine_core/enums.h"
#include "renderer/common.h"
#include "renderer/module_objects.h"
#include "renderer/cpu_buffer.h"

namespace ad_astris::renderer::impl
{
	struct IndirectBuffers
	{
		rhi::Buffer* indirectBuffer{ nullptr };
		rhi::Buffer* modelInstanceIDBuffer{ nullptr };
		uint32_t cullingParamsIndex{ 0 };
	};
	
	class CullingSubmanager
	{
		public:
			CullingSubmanager();

			void update(rhi::CommandBuffer& cmdBuffer);
			void cleanup_after_update();
			bool need_update();

			void add_model(
				ecs::Entity entity,
				ecore::StaticModel* model,
				uint32_t vertexOffset,
				uint32_t indexOffset);

			// Need to add light source entities to cull objects for shadow maps
			void add_light(ecs::Entity entity);
			void add_culling_instance(ecs::Entity entity, uint32_t objectIndex, UUID modelUUID);

			template<typename ...ARGS>
			FORCE_INLINE void add_scene_entity_filter(const ecs::EntityFilter<ARGS...>& filter)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it == _sceneCullingContextByEntityFilterHash.end())
				{
					uint32_t index = _cullingParamsCpuBuffer->get_element_count();
					CullingParams params;
					_cullingParamsCpuBuffer->push_back(&params, 1);
					_sceneCullingContextByEntityFilterHash[filter.get_requirements_hash()].init(filter, ecore::MAIN_CAMERA, index);
					_sceneEntityFilters.emplace_back(new ecs::EntityFilter<ARGS...>(filter));
				}
			}

			template<typename ...ARGS>
			FORCE_INLINE void add_shadows_entity_filter(const ecs::EntityFilter<ARGS...>& filter)
			{
				auto it = _shadowsCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it == _shadowsCullingContextByEntityFilterHash.end())
				{
					_shadowsCullingContextByEntityFilterHash[filter.get_requirements_hash()].init(filter);
					_shadowsEntityFilters.emplace_back(new ecs::EntityFilter<ARGS...>(filter));
				}
			}

			FORCE_INLINE rhi::Buffer* get_culling_params_buffer()
			{
				return _cullingParamsBuffer;
			}
		
			FORCE_INLINE IndirectBuffers get_scene_indirect_buffers(const ecs::IEntityFilter& filter, ecore::CameraIndex cameraIndex)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it == _sceneCullingContextByEntityFilterHash.end())
				{
					LOG_FATAL("CullingSubmanager::get_scene_indirect_buffers(): No filter")
				}
				auto it2 = it->second.indirectBuffersByCameraIndex.find(cameraIndex);
				if (it2 == it->second.indirectBuffersByCameraIndex.end())
				{
					LOG_FATAL("CullingSubmanager::get_scene_indirect_buffers(): No camera with index {}", cameraIndex)
				}
				return it2->second;
			}
		
			FORCE_INLINE std::vector<IndirectBuffers> get_light_indirect_buffers(ecs::Entity lightSource)
			{
				std::vector<IndirectBuffers> buffers;
				for (auto& pair : _shadowsCullingContextByEntityFilterHash)
				{
					ShadowsCullingContext& cullingContext = pair.second;
					auto it = cullingContext.indirectBuffersByLightSource.find(lightSource);
					if (it != cullingContext.indirectBuffersByLightSource.end())
					{
						buffers.push_back(it->second);
					}
				}
				return buffers;
			}
		
			FORCE_INLINE rhi::Buffer* get_culling_indices_buffer(const ecs::IEntityFilter& filter)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it != _sceneCullingContextByEntityFilterHash.end())
				{
					return it->second.cullingInstanceIndicesBuffer;
				}

				auto it2 = _shadowsCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it2 != _shadowsCullingContextByEntityFilterHash.end())
				{
					return it2->second.cullingInstanceIndicesBuffer;
				}
				
				LOG_FATAL("CullingSubmanager::get_culling_indices_buffer(): No filter")
			}
		
			FORCE_INLINE uint32_t get_instance_count(const ecs::IEntityFilter& filter)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it != _sceneCullingContextByEntityFilterHash.end())
				{
					return it->second.instanceCount;
				}

				auto it2 = _shadowsCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it2 != _shadowsCullingContextByEntityFilterHash.end())
				{
					return it2->second.instanceCount;
				}
				LOG_FATAL("CullingSubmanager::get_instance_count(): No filter")
			}
		
			FORCE_INLINE uint32_t get_indirect_command_count(const ecs::IEntityFilter& filter)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it != _sceneCullingContextByEntityFilterHash.end())
				{
					return it->second.indirectCommands->get_element_count();
				}

				auto it2 = _shadowsCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it2 != _shadowsCullingContextByEntityFilterHash.end())
				{
					return it2->second.indirectCommands->get_element_count();
				}
				LOG_FATAL("CullingSubmanager::get_main_culling_indices_buffer(): No filter")
			}
			// FORCE_INLINE uint32_t get_batch_instance_count(uint32_t batchIndex) { return _indirectCommands[batchIndex].instanceCount; }

			template<typename ...ARGS>
			FORCE_INLINE std::string get_indirect_buffer_name(const ecs::EntityFilter<ARGS...>& filter)
			{
				return filter.get_name() + INDIRECT_BUFFER_NAME;
			}

			template<typename ...ARGS>
			FORCE_INLINE std::string get_model_instance_id_buffer_name(const ecs::EntityFilter<ARGS...>& filter)
			{
				return filter.get_name() + MODEL_INSTANCE_ID_BUFFER_NAME;
			}
		
		private:
			inline static const std::string INDIRECT_BUFFER_NAME = "IndirectBuffer";
			inline static const std::string CPU_INDIRECT_BUFFER_NAME = "CpuIndirectBuffer";
			inline static const std::string MODEL_INSTANCE_ID_BUFFER_NAME = "ModelInstanceIdBuffer";
			inline static const std::string CULLING_INDICES_BUFFER_NAME = "CullingIndicesBuffer";
			inline static const std::string CPU_CULLING_INDICES_BUFFER_NAME = "CpuCullingIndicesBuffer";
			inline static const std::string CPU_CULLING_PARAMS_BUFFER_NAME = "CpuCullingParamsBuffer";
			inline static const std::string CULLING_PARAMS_BUFFER_NAME = "CullingParamsBuffer";
		
			struct BaseCullingContext
			{
				rhi::Buffer* cullingInstanceIndicesBuffer{ nullptr };
				std::unique_ptr<CpuBuffer<DrawIndexedIndirectCommand>> indirectCommands;
				std::unique_ptr<CpuBuffer<CullingInstanceIndices>> cullingInstanceIndices;
				std::unordered_map<UUID, size_t> indirectBatchIndexByModelUUID;
				uint32_t instanceCount{ 0 };
				std::string entityFilterName;
			};
		
			struct SceneCullingContext : BaseCullingContext
			{
				std::unordered_map<ecore::CameraIndex, IndirectBuffers> indirectBuffersByCameraIndex;
				
				template<typename ...ARGS>
				void init(const ecs::EntityFilter<ARGS...>& filter, ecore::CameraIndex cameraIndex, uint32_t cullingParamsIndex)
				{
					std::string entityFilterName = filter.get_name();
					IndirectBuffers& indirectBuffers = indirectBuffersByCameraIndex[cameraIndex];
					indirectBuffers.cullingParamsIndex = cullingParamsIndex;
	
					indirectCommands = std::make_unique<CpuBuffer<DrawIndexedIndirectCommand>>(
						entityFilterName + CPU_INDIRECT_BUFFER_NAME,
						sizeof(DrawIndexedIndirectCommand) * INDIRECT_BATCH_INIT_NUMBER);
					cullingInstanceIndices = std::make_unique<CpuBuffer<CullingInstanceIndices>>(
						entityFilterName + CPU_CULLING_INDICES_BUFFER_NAME,
						sizeof(CullingInstanceIndices) * MODEL_INSTANCES_INIT_NUMBER);

					indirectBuffers.indirectBuffer = RENDERER_RESOURCE_MANAGER()->allocate_indirect_buffer(
						entityFilterName + INDIRECT_BUFFER_NAME,
						sizeof(DrawIndexedIndirectCommand) * INDIRECT_BATCH_INIT_NUMBER);
					indirectBuffers.modelInstanceIDBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
						entityFilterName + MODEL_INSTANCE_ID_BUFFER_NAME,
						sizeof(RendererModelInstanceID) * MODEL_INSTANCES_INIT_NUMBER);
					cullingInstanceIndicesBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
						entityFilterName + CULLING_INDICES_BUFFER_NAME,
						sizeof(CullingInstanceIndices) * MODEL_INSTANCES_INIT_NUMBER);
				}
			};

			struct ShadowsCullingContext : BaseCullingContext
			{
				std::unordered_map<ecs::Entity, IndirectBuffers> indirectBuffersByLightSource;
				void add_light(ecs::Entity light, uint32_t cullingParamsIndex);
			};

			ecs::Entity cameras[ecore::CAMERA_COUNT];
			std::vector<std::unique_ptr<ecs::IEntityFilter>> _sceneEntityFilters;
			std::unordered_map<size_t, SceneCullingContext> _sceneCullingContextByEntityFilterHash;
			std::vector<std::unique_ptr<ecs::IEntityFilter>> _shadowsEntityFilters;
			std::unordered_map<size_t, ShadowsCullingContext> _shadowsCullingContextByEntityFilterHash;
			std::unique_ptr<CpuBuffer<CullingParams>> _cullingParamsCpuBuffer;
			rhi::Buffer* _cullingParamsBuffer;

			SceneCullingContext* get_scene_culling_context(ecs::Entity entity);

			void subscribe_to_events();

			void update_cpu_arrays();
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
	};
}
