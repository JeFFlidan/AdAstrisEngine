#pragma once

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
		
		private:
			GraphicsAPI _graphicsAPI{ GraphicsAPI::UNDEFINED };

			struct
			{
				uint64_t width;
				uint64_t height;
				bool useVSync;
				bool useTripleBuffering;
			} _swapChainDesc;
				
			void serialize_graphics_api(Section& section);
			void deserialize_graphics_api(Section& section);
			void serialize_swap_chain_desc(Section& section);
			void deserialize_swap_chain_desc(Section& section);
	};
}