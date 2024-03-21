#pragma once

#include "common.h"
#include "shader_interop_renderer.h"
#include "engine_core/basic_components.h"
#include "../module_objects.h"
#include "../renderer_resource_collection.h"

namespace ad_astris::renderer::impl
{
	class EntitySubmanager
	{
		public:
			EntitySubmanager();

			void update(rhi::CommandBuffer& cmdBuffer);
			void cleanup_after_update();
			bool need_update();

			rhi::Buffer* get_entity_buffer() const
			{
				if (_rendererEntities->is_gpu_collection())
					return _rendererEntities->get_gpu_buffer();
				return _rendererEntities->get_mapped_buffer();
			}

			void add_light_entity(ecs::Entity entity)
			{
				_engineEntities.push_back(entity);
			}

			uint32_t get_light_count() { return _pointLightCount + _spotLightCount + _directionalLightCount; }
		
		private:
			const std::string RENDERER_ENTITY_BUFFER_NAME = "RendererEntityBuffer";
			const std::string MATRIX_BUFFER_NAME = "MatrixBuffer";

			std::unique_ptr<RendererResourceCollection<RendererEntity>> _rendererEntities{ nullptr };
			std::mutex _rendererEntitiesMutex;
			std::vector<ecs::Entity> _engineEntities;
			std::atomic_uint32_t _pointLightCount{ 0 };
			std::atomic_uint32_t _spotLightCount{ 0 };
			std::atomic_uint32_t _directionalLightCount{ 0 };
		
			bool _areGPUBuffersAllocated{ false };
		
			void subscribe_to_events();

			void allocate_buffers();
			void update_gpu_buffer(rhi::CommandBuffer& cmd);
			void update_cpu_arrays();

			void setup_light(const ecs::Entity& entity);
		
			void setup_point_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_directional_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_spot_lights_matrices(tasks::TaskGroup& taskGroup);
	};

	IMPLEMENT_SUBMANAGER_EVENT(LightSubmanager)
}