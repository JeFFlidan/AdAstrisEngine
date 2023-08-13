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
		
		private:
			GraphicsAPI _graphicsAPI{ GraphicsAPI::UNDEFINED };
				
			void serialize_graphics_api(Section& section);
			void deserialize_graphics_api(Section& section);
	};
}