#include "engine_settings.h"

#include "profiler/logger.h"

using namespace ad_astris;
using namespace engine;

void EngineSettings::serialize(io::FileSystem* fileSystem)
{
	_rendererSettings.serialize(_config);

	_config.save(fileSystem);
}

void EngineSettings::deserialize(const io::URI& engineConfigFilePath)
{
	_config.load_from_file(engineConfigFilePath);

	_rendererSettings.deserialize(_config);

	_config.unload();
}

void RendererSettings::serialize(Config& config)
{
	Section section("RendererSettings");
	serialize_graphics_api(section);

	config.set_section(section);
}

void RendererSettings::deserialize(Config& config)
{
	Section section = config.get_section("RendererSettings");
	deserialize_graphics_api(section);
}

void RendererSettings::serialize_graphics_api(Section& section)
{
	switch (_graphicsAPI)
	{
		case GraphicsAPI::VULKAN:
			section.set_option("GraphicsAPI", "Vulkan");
			break;
		case GraphicsAPI::D3D12:
			section.set_option( "GraphicsAPI", "D3D12");
			break;
		case GraphicsAPI::UNDEFINED:
			LOG_FATAL("EngineSettings::serialize(): Graphics API can't be undefined")
			break;
	}
}

void RendererSettings::deserialize_graphics_api(Section& section)
{
	std::string strGraphicsAPI = section.get_option_value<std::string>("GraphicsAPI");

	if (strGraphicsAPI == "Vulkan")
		_graphicsAPI = GraphicsAPI::VULKAN;
	if (strGraphicsAPI == "D3D12")
		_graphicsAPI = GraphicsAPI::D3D12;
}
