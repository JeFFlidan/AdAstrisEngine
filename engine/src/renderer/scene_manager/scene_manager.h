#pragma once

#include "scene_submanager_base.h"
#include "model_submanager.h"
#include "light_submanager.h"
#include "rhi/engine_rhi.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "core/reflection.h"
#include "ecs.h"

namespace ad_astris::renderer::impl
{
	class SceneManager
	{
		public:
			SceneManager(SceneManagerInitializationContext& initContext);
			~SceneManager();

			void setup_global_buffers();
			void execute_transfer_operations();
		
			rhi::Buffer* get_vertex_buffer_f32pntc()
			{
				return get_model_submanager()->get_vertex_buffer_f32pntc();
			}

			rhi::Buffer* get_index_buffer_f32pntc()
			{
				return get_model_submanager()->get_index_buffer_f32pntc();
			}

			rhi::Buffer* get_output_plane_vertex_buffer()
			{
				return get_model_submanager()->get_output_plane_vertex_buffer();
			}

			rhi::Buffer* get_point_light_storage_buffer()
			{
				return get_light_submanager()->get_point_light_storage_buffer();
			}

			rhi::Buffer* get_directional_light_storage_buffer()
			{
				return get_light_submanager()->get_directional_light_storage_buffer();
			}

			rhi::Buffer* get_spot_light_storage_buffer()
			{
				return get_light_submanager()->get_spot_light_storage_buffer();
			}
		
		private:
			rhi::IEngineRHI* _rhi{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };

			std::unordered_map<std::string, std::unique_ptr<SceneSubmanagerBase>> _submanagerByItsName;
			std::unordered_set<std::string> _submanagersToUpdate;
		
			// AllocatedBuffer _pointLightStorageBuffer;
			// AllocatedBuffer _directionalLightStorageBuffer;
			// AllocatedBuffer _spotLightStorageBuffer;
			rhi::CommandBuffer _transferCmdBuffer;
			bool _wasCommandBufferBegun{ false };
		
			// ObjectCPUCollection<PointLight> _pointLightCollection;
			// ObjectCPUCollection<DirectionalLight> _directionalLightCollection;
			// ObjectCPUCollection<SpotLight> _spotLightCollection;

			void subscribe_to_events();

			ModelSubmanager* get_model_submanager()
			{
				return static_cast<ModelSubmanager*>(_submanagerByItsName[get_type_name<ModelSubmanager>()].get());
			}

			LightSubmanager* get_light_submanager()
			{
				return static_cast<LightSubmanager*>(_submanagerByItsName[get_type_name<LightSubmanager>()].get());
			}

			template<typename T>
			void add_submanager(SceneSubmanagerInitializationContext& initContext)
			{
				_submanagerByItsName[get_type_name<T>()] = std::move(std::make_unique<T>(initContext));
			}
	};
}
