#pragma once

#include "material_submanager.h"
#include "culling_submanager.h"
#include "model_submanager.h"
#include "entity_submanager.h"

namespace ad_astris::renderer::impl
{
	class SceneManager
	{
		public:
			SceneManager();
			~SceneManager();
		
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
		
			uint32_t get_light_count() { return _entitySubmanager->get_light_count(); }
			CullingSubmanager* get_culling_submanager() { return _cullingSubmanager.get(); }
		
		private:
			std::unique_ptr<ModelSubmanager> _modelSubmanager{ nullptr };
			std::unique_ptr<MaterialSubmanager> _materialSubmanager{ nullptr };
			std::unique_ptr<EntitySubmanager> _entitySubmanager{ nullptr };
			std::unique_ptr<CullingSubmanager> _cullingSubmanager{ nullptr };

			void subscribe_to_events();
	};
}
