#pragma once

#include "scene_submanager_base.h"
#include "model_submanager.h"
#include "entity_submanager.h"
#include "rhi/engine_rhi.h"
#include "renderer/enums.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "material_submanager.h"

namespace ad_astris::renderer::impl
{
	class SceneManager
	{
		public:
			SceneManager(SceneManagerInitializationContext& initContext);
			~SceneManager();

			void update_per_frame_cpu_data();
			void setup_global_buffers();
		
			rhi::Buffer* get_vertex_buffer_f32pntc()
			{
				return _modelSubmanager->get_vertex_buffer_f32pntc();
			}

			rhi::Buffer* get_index_buffer_f32pntc()
			{
				return _modelSubmanager->get_index_buffer_f32pntc();
			}

			rhi::Buffer* get_output_plane_vertex_buffer()
			{
				return _modelSubmanager->get_output_plane_vertex_buffer();
			}

			rhi::Buffer* get_model_instance_storage_buffer()
			{
				return _modelSubmanager->get_model_instance_buffer();
			}

			rhi::Buffer* get_material_storage_buffer()
			{
				return _materialSubmanager->get_material_buffer();
			}

			rhi::Buffer* get_entity_storage_buffer()
			{
				return _entitySubmanager->get_entity_buffer();
			}

			rhi::Sampler get_sampler(SamplerType samplerType)
			{
				return _materialSubmanager->get_sampler(samplerType);
			}
		
			IndirectBufferDesc* get_indirect_buffer_desc() { return _modelSubmanager->get_indirect_buffer_desc(); }

			uint32_t get_light_count() { return _entitySubmanager->get_light_count(); }
		
		private:
			rhi::IEngineRHI* _rhi{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
			rcore::IRendererResourceManager* _rendererResourceManager{ nullptr };

			std::unique_ptr<ModelSubmanager> _modelSubmanager;
			std::unique_ptr<MaterialSubmanager> _materialSubmanager;
			std::unique_ptr<EntitySubmanager> _entitySubmanager;

			void subscribe_to_events();
	};
}
