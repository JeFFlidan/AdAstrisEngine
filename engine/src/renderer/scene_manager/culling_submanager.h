#pragma once

#include "common.h"
#include "shader_interop_renderer.h"
#include "engine_core/basic_components.h"
#include "engine_core/enums.h"
#include "renderer/common.h"
#include "renderer/module_objects.h"
#include "renderer/renderer_array.h"
#include "renderer/renderer_resource_collection.h"
#include "core/non_copyable_non_movable.h"

namespace ad_astris::renderer::impl
{
	class DepthPyramid
	{
		public:
			DepthPyramid(size_t entityFilterHash);

			rhi::Texture* get_texture() const
			{
				return RENDERER_RESOURCE_MANAGER()->get_texture(get_str_with_id(DEPTH_PYRAMID_NAME));
			}

			rhi::TextureView* get_texture_view() const
			{
				return RENDERER_RESOURCE_MANAGER()->get_texture_view(get_str_with_id(DEPTH_PYRAMID_NAME));
			}
		
			rhi::TextureView* get_mipmap(size_t index) const
			{
				return RENDERER_RESOURCE_MANAGER()->get_texture_view(get_str_with_id(DEPTH_PYRAMID_MIPMAP_NAME) + std::to_string(index));
			}
		
			uint32_t get_width() const { return _width; }
			uint32_t get_height() const { return _height; }
			uint32_t get_mip_levels() const { return _mipLevels; }
			std::string get_depth_pyramid_name() const { return get_str_with_id(DEPTH_PYRAMID_NAME); }
		
		private:
			inline static constexpr const char* DEPTH_PYRAMID_NAME = "DepthPyramid";
			inline static constexpr const char* DEPTH_PYRAMID_MIPMAP_NAME = "DepthPyramidMipmap";
		
			uint32_t _width{ 0 };
			uint32_t _height{ 0 };
			uint32_t _mipLevels{ 0 };
			size_t _entityFilterHash{ 0 };
				
			std::string get_str_with_id(const std::string& str) const
			{
				return std::to_string(_entityFilterHash) + str;
			}
	};
	
	struct IndirectBuffers : public NonCopyableNonMovable
	{
		rhi::Buffer* indirectBuffer{ nullptr };
		rhi::Buffer* modelInstanceIDBuffer{ nullptr };
		std::unique_ptr<DepthPyramid> depthPyramid{ nullptr };
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
		
			FORCE_INLINE IndirectBuffers& get_scene_indirect_buffers(const ecs::IEntityFilter& filter, ecore::CameraIndex cameraIndex)
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
		
			FORCE_INLINE std::vector<IndirectBuffers*> get_light_indirect_buffers(ecs::Entity lightSource)
			{
				std::vector<IndirectBuffers*> buffers;
				for (auto& pair : _shadowsCullingContextByEntityFilterHash)
				{
					ShadowsCullingContext& cullingContext = pair.second;
					auto it = cullingContext.indirectBuffersByLightSource.find(lightSource);
					if (it != cullingContext.indirectBuffersByLightSource.end())
					{
						buffers.push_back(&it->second);
					}
				}
				return buffers;
			}
		
			FORCE_INLINE rhi::Buffer* get_culling_indices_buffer(const ecs::IEntityFilter& filter)
			{
				auto it = _sceneCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it != _sceneCullingContextByEntityFilterHash.end())
				{
					return it->second.cullingInstanceIndices->get_gpu_buffer();
				}

				auto it2 = _shadowsCullingContextByEntityFilterHash.find(filter.get_requirements_hash());
				if (it2 != _shadowsCullingContextByEntityFilterHash.end())
				{
					return it2->second.cullingInstanceIndices->get_gpu_buffer();
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

			template<typename ...ARGS>
			FORCE_INLINE std::string get_indirect_buffer_name(const ecs::EntityFilter<ARGS...>& filter)
			{
				return std::to_string(filter.get_requirements_hash()) + INDIRECT_BUFFER_NAME;
			}

			template<typename ...ARGS>
			FORCE_INLINE std::string get_model_instance_id_buffer_name(const ecs::EntityFilter<ARGS...>& filter)
			{
				return std::to_string(filter.get_requirements_hash()) + MODEL_INSTANCE_ID_BUFFER_NAME;
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
				std::unique_ptr<RendererArray<DrawIndexedIndirectCommand>> indirectCommands{ nullptr };
				std::unique_ptr<RendererResourceCollection<CullingInstanceIndices>> cullingInstanceIndices{ nullptr };
				std::unordered_map<UUID, size_t> indirectBatchIndexByModelUUID;
				uint32_t instanceCount{ 0 };
				size_t entityFilterHash{ 0 };
			};
		
			struct SceneCullingContext : BaseCullingContext
			{
				std::unordered_map<ecore::CameraIndex, IndirectBuffers> indirectBuffersByCameraIndex;
				
				template<typename ...ARGS>
				void init(const ecs::EntityFilter<ARGS...>& filter, ecore::CameraIndex cameraIndex, uint32_t cullingParamsIndex)
				{
					entityFilterHash = filter.get_requirements_hash();
					IndirectBuffers& indirectBuffers = indirectBuffersByCameraIndex[cameraIndex];
					indirectBuffers.cullingParamsIndex = cullingParamsIndex;
	
					indirectCommands = std::make_unique<RendererArray<DrawIndexedIndirectCommand>>(
						std::to_string(entityFilterHash) + CPU_INDIRECT_BUFFER_NAME,
						sizeof(DrawIndexedIndirectCommand) * INDIRECT_BATCH_INIT_NUMBER);
					cullingInstanceIndices = std::make_unique<RendererResourceCollection<CullingInstanceIndices>>(
						std::to_string(entityFilterHash) + CPU_CULLING_INDICES_BUFFER_NAME,
						std::to_string(entityFilterHash) + CULLING_INDICES_BUFFER_NAME,
						rhi::ResourceUsage::STORAGE_BUFFER,
						sizeof(CullingInstanceIndices) * MODEL_INSTANCES_INIT_NUMBER);

					indirectBuffers.indirectBuffer = RENDERER_RESOURCE_MANAGER()->allocate_indirect_buffer(
						std::to_string(entityFilterHash) + INDIRECT_BUFFER_NAME,
						sizeof(DrawIndexedIndirectCommand) * INDIRECT_BATCH_INIT_NUMBER);
					indirectBuffers.modelInstanceIDBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
						std::to_string(entityFilterHash) + MODEL_INSTANCE_ID_BUFFER_NAME,
						sizeof(RendererModelInstanceID) * MODEL_INSTANCES_INIT_NUMBER);
					
					if (RENDERER_SUBSETTINGS()->get_scene_culling_settings().isOcclusionCullingEnabled && !indirectBuffers.depthPyramid)
						indirectBuffers.depthPyramid = std::make_unique<DepthPyramid>(entityFilterHash);
				}
			};

			struct ShadowsCullingContext : BaseCullingContext
			{
				std::unordered_map<ecs::Entity, IndirectBuffers> indirectBuffersByLightSource;
				void add_light(ecs::Entity light, uint32_t cullingParamsIndex);
			};

			ecs::Entity _cameras[ecore::CAMERA_COUNT];
			std::vector<std::unique_ptr<ecs::IEntityFilter>> _sceneEntityFilters;
			std::unordered_map<size_t, SceneCullingContext> _sceneCullingContextByEntityFilterHash;
			std::vector<std::unique_ptr<ecs::IEntityFilter>> _shadowsEntityFilters;
			std::unordered_map<size_t, ShadowsCullingContext> _shadowsCullingContextByEntityFilterHash;
			std::unique_ptr<RendererArray<CullingParams>> _cullingParamsCpuBuffer;
			rhi::Buffer* _cullingParamsBuffer;

			SceneCullingContext* get_scene_culling_context(ecs::Entity entity);

			void subscribe_to_events();

			void update_cpu_arrays(rhi::CommandBuffer& cmd);
			void update_gpu_buffers(rhi::CommandBuffer& cmd);
	};
}
