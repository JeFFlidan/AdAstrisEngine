#pragma once

#include "scene_submanager_base.h"
#include "shader_interop_renderer.h"

namespace ad_astris::renderer::impl
{
	class MaterialSubmanager : public SceneSubmanagerBase
	{
		public:
			MaterialSubmanager(SceneSubmanagerInitializationContext& initContext);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_after_update() override;
			virtual bool need_update() override;

			void add_cpu_opaque_material_uuid(UUID uuid) { _gpuOpaqueMaterialIndexByCPUMaterialUUID.insert({ uuid, 0 }); }
			uint32_t get_gpu_material_index(UUID cpuMaterialUUID);

			rhi::Buffer* get_material_buffer() { return _rendererResourceManager->get_buffer(MATERIAL_BUFFER_NAME); }
			rhi::Sampler get_sampler(SamplerType type) { return _samplers[type]; }

		private:
			const std::string MATERIAL_BUFFER_NAME = "material_buffer";
			
			std::vector<RendererMaterial> _rendererMaterials;
			std::unordered_map<UUID, uint32_t> _gpuOpaqueMaterialIndexByCPUMaterialUUID;
			std::unordered_map<UUID, rhi::TextureView*> _gpuTextureViewByCPUTextureUUID;
			tasks::TaskGroup _textureLoadingTaskGroup;
			rhi::Sampler _samplers[SAMPLER_COUNT];

			bool _areGPUBuffersAllocated{ false };
		
			virtual void subscribe_to_events() override;
			void update_cpu_arrays(rhi::CommandBuffer& cmd);
			void allocate_gpu_buffers();
			rhi::TextureView* allocate_2d_texture(rhi::CommandBuffer& cmd, UUID uuid);
			void create_samplers();
	};
}
