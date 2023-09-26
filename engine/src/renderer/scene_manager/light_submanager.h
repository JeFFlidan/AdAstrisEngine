#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "renderer/common.h"
#include "engine_core/basic_components.h"

namespace ad_astris::renderer::impl
{
	class LightSubmanager : public SceneSubmanagerBase
	{
		public:
			LightSubmanager(SceneSubmanagerInitializationContext& initContext);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_after_update() override;
			virtual bool need_update() override;

			rhi::Buffer* get_point_light_storage_buffer()
			{
				return _rendererResourceManager->get_buffer(POINT_LIGHT_BUFFER_NAME);
			}

			rhi::Buffer* get_directional_light_storage_buffer()
			{
				return _rendererResourceManager->get_buffer(DIR_LIGHT_BUFFER_NAME);
			}

			rhi::Buffer* get_spot_light_storage_buffer()
			{
				return _rendererResourceManager->get_buffer(SPOT_LIGHT_BUFFER_NAME);
			}

		private:
			const std::string POINT_LIGHT_BUFFER_NAME = "point_light_buffer";
			const std::string SPOT_LIGHT_BUFFER_NAME = "spot_light_buffer";
			const std::string DIR_LIGHT_BUFFER_NAME = "dir_light_buffer";
		
			struct
			{
				ObjectCPUCollection<PointLight> pointLights;
				ObjectCPUCollection<DirectionalLight> directionalLights;
				ObjectCPUCollection<SpotLight> spotLights;
			} _cpuCollections;
		
			struct PointLightTemp
			{
				uint32_t pointLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct DirectionalLightTemp
			{
				uint32_t directionalLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct SpotLightTemp
			{
				uint32_t spotLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct
			{
				std::vector<PointLightTemp> pointLights;
				std::vector<DirectionalLightTemp> directionalLights;
				std::vector<SpotLightTemp> spotLights;
			} _recentlyCreated;

			bool _areGPUBuffersAllocated{ false };
		
			virtual void subscribe_to_events() override;

			void allocate_gpu_buffers();
			void update_gpu_buffer(rhi::CommandBuffer& cmd);

			void setup_lights();
		
			void setup_point_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_directional_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_spot_lights_matrices(tasks::TaskGroup& taskGroup);
	};

	IMPLEMENT_SUBMANAGER_EVENT(LightSubmanager)
}