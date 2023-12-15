﻿#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "shader_interop_renderer.h"
#include "engine_core/basic_components.h"

namespace ad_astris::renderer::impl
{
	class EntitySubmanager : public SceneSubmanagerBase
	{
		public:
			EntitySubmanager(SceneSubmanagerInitializationContext& initContext);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_after_update() override;
			virtual bool need_update() override;

			rhi::Buffer* get_entity_buffer()
			{
				return _rendererResourceManager->get_buffer(RENDERER_ENTITY_BUFFER_NAME);
			}

			void add_light_entity(ecs::Entity entity)
			{
				_engineEntities.push_back(entity);
			}

			uint32_t get_light_count() { return _pointLightCount + _spotLightCount + _directionalLightCount; }
		
		private:
			const std::string RENDERER_ENTITY_BUFFER_NAME = "renderer_entity_buffer";
			const std::string MATRIX_BUFFER_NAME = "matrix_buffer";

			std::vector<RendererEntity> _rendererEntities;
			std::mutex _rendererEntitiesMutex;
			std::vector<ecs::Entity> _engineEntities;
			std::atomic_uint32_t _pointLightCount{ 0 };
			std::atomic_uint32_t _spotLightCount{ 0 };
			std::atomic_uint32_t _directionalLightCount{ 0 };
		
			bool _areGPUBuffersAllocated{ false };
		
			virtual void subscribe_to_events() override;

			void allocate_gpu_buffers();
			void update_gpu_buffer(rhi::CommandBuffer& cmd);
			void update_cpu_arrays();

			void setup_light(ecs::Entity entity);
		
			void setup_point_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_directional_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_spot_lights_matrices(tasks::TaskGroup& taskGroup);
	};

	IMPLEMENT_SUBMANAGER_EVENT(LightSubmanager)
}