#pragma once

#include "engine_core/subsettings.h"

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