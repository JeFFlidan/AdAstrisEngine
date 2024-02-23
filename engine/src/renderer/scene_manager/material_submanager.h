#pragma once

#include "ecs/ecs.h"
#include "renderer/module_objects.h"
#include "shader_interop_renderer.h"

namespace ad_astris::renderer::impl
{
	class MaterialSubmanager
	{
		public:
			MaterialSubmanager();

			void update(rhi::CommandBuffer& cmdBuffer);
			void cleanup_after_update();
			bool need_update();

			void add_material(ecs::Entity entity);
			uint32_t get_gpu_material_index(rhi::CommandBuffer& cmd, UUID cpuMaterialUUID);

			rhi::Buffer* get_material_buffer() { return RENDERER_RESOURCE_MANAGER()->get_buffer(MATERIAL_BUFFER_NAME); }
			rhi::Sampler get_sampler(SamplerType type) { return _samplers[type]; }

		private:
			const std::string MATERIAL_BUFFER_NAME = "material_buffer";
			
			std::vector<RendererMaterial> _rendererMaterials;
			std::unordered_map<UUID, uint32_t> _gpuOpaqueMaterialIndexByCPUMaterialUUID;
			std::unordered_map<UUID, rhi::TextureView*> _gpuTextureViewByCPUTextureUUID;
			tasks::TaskGroup _textureLoadingTaskGroup;
			rhi::Sampler _samplers[SAMPLER_COUNT];

			bool _areGPUBuffersAllocated{ false };
		
			void subscribe_to_events();
			void update_cpu_arrays(rhi::CommandBuffer& cmd);
			void allocate_gpu_buffers();
			rhi::TextureView* allocate_2d_texture(rhi::CommandBuffer& cmd, UUID uuid);
			void create_samplers();
			void create_gpu_material(rhi::CommandBuffer& cmd, UUID cpuMaterialUUID);
	};
}
