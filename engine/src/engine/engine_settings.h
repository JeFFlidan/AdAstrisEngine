#pragma once

#include "core/config_base.h"
#include "file_system/file_system.h"

namespace ad_astris::engine
{
	enum class GraphicsAPI
	{
		UNDEFINED,
		VULKAN,
		D3D12
	};

	class RendererSettings
	{
		public:
			void serialize(Config& config);
			void deserialize(Config& config);

			GraphicsAPI get_graphics_api()
			{
				return _graphicsAPI;
			}
		
		private:
			GraphicsAPI _graphicsAPI{ GraphicsAPI::UNDEFINED };
			
			void serialize_graphics_api(Section& section);
			void deserialize_graphics_api(Section& section);
	};
	
	class EngineSettings
	{
		public:
			EngineSettings() = default;

			void serialize(io::FileSystem* fileSystem);
			void deserialize(const io::URI& engineConfigFilePath);

			RendererSettings& get_renderer_settings()
			{
				return _rendererSettings;
			}

		private:
			Config _config;
			RendererSettings _rendererSettings;
	};
}