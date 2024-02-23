#pragma once

#include "engine_core/uuid.h"
#include "engine_core/subsettings.h"
#include "profiler/logger.h"

namespace ad_astris::ecore
{
	enum class GraphicsAPI
	{
		UNDEFINED,
		VULKAN,
		D3D12
	};

	struct SceneCullingSettings
	{
		uint64_t drawDistance;
		bool isFrustumCullingEnabled;
		bool isOcclusionCullingEnabled;
		float lodBase;
		float lodStep;
	};

	struct ShadowsCullingSettings : public SceneCullingSettings
	{
		bool isAabbCheckEnabled;
	};

	class RendererSubsettings : public ISubsettings
	{
		public:
			SUBSETTINGS_TYPE_DECL(RendererSubsettings)
		
			virtual void serialize(Config& config) override;
			virtual void deserialize(Section& section) override;
			virtual void setup_default_values() override;

			void set_graphics_api(GraphicsAPI graphicsAPI)
			{
				if (graphicsAPI == GraphicsAPI::UNDEFINED)
					LOG_FATAL("RendererSubsettings::set_graphics_api(): Graphics API can't be UNDEFINED")

				_graphicsAPI = graphicsAPI;
			}

			GraphicsAPI get_graphics_api()
			{
				return _graphicsAPI;
			}

			bool is_triple_buffering_used()
			{
				return _swapChainDesc.useTripleBuffering;
			}

			bool is_vsync_used()
			{
				return _swapChainDesc.useVSync;
			}

			uint32_t get_render_area_width()
			{
				return _swapChainDesc.width;
			}

			uint32_t get_render_area_height()
			{
				return _swapChainDesc.height;
			}

			void set_default_material_uuid(UUID uuid)
			{
				_defaultMaterialUUID = uuid;
			}

			UUID get_default_material_uuid()
			{
				return _defaultMaterialUUID;
			}

			const SceneCullingSettings& get_scene_culling_settings()
			{
				return _sceneCullingSettings;
			}

			const ShadowsCullingSettings& get_shadow_maps_culling()
			{
				return _shadowsCulllingSettings;
			}
		
		private:
			GraphicsAPI _graphicsAPI{ GraphicsAPI::UNDEFINED };
			SceneCullingSettings _sceneCullingSettings;
			ShadowsCullingSettings _shadowsCulllingSettings;

			struct
			{
				uint64_t width;
				uint64_t height;
				bool useVSync;
				bool useTripleBuffering;
			} _swapChainDesc;

			UUID _defaultMaterialUUID;
				
			void serialize_graphics_api(Section& section);
			void deserialize_graphics_api(Section& section);
			void serialize_swap_chain_desc(Section& section);
			void deserialize_swap_chain_desc(Section& section);
			void serialize_scene_culling_settings(Section& section);
			void deserialize_scene_culling_settings(Section& section);
			void serialize_shadows_culling_settings(Section& section);
			void deserialize_shadows_culling_settings(Section& section);
	};
}